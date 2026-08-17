/*!
 * @brief     AD9986 control-plane example application for the
 *            Raspberry Pi 5 + Lattice FPGA platform.
 *
 *            Host        : Raspberry Pi 5 (SPI master, Linux spidev)
 *            SPI0 CS0    : JESD IP register interface (inside the Lattice FPGA)
 *            SPI0 CS1    : AD9986 (proxied by the Lattice FPGA)
 *            SPI1 CS0    : HMC7044 clock generator
 *
 *            This example exercises the SPI configuration plane only:
 *               - reads the JESD IP version through the reused adi_ads9_* API
 *                 (which now reaches the FPGA over SPI0 CS0);
 *               - resets, initializes and identifies the AD9986 over SPI0 CS1;
 *               - identifies the HMC7044 over SPI1.
 *
 *            JESD204 DMA data capture is not part of this platform.
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <stdint.h>
#include "adi_ad9986.h"
#include "adi_ad9986_hal.h"
#include "adi_hmc7044.h"
#include "adi_cms_api_common.h"
#include "adi_ads9.h"
#include "lattice.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>

/* Forward declarations for JESD functions not yet exposed in a header */
int32_t adi_ad9986_jesd_rx_bit_rate_get(adi_ad9986_device_t *device, uint64_t *bit_rate_bps);
int32_t adi_ad9986_jesd_tx_bit_rate_get(adi_ad9986_device_t *device, adi_ad9986_jesd_link_select_e link, uint64_t *bit_rate_bps);
int32_t adi_ad9986_jesd_tx_gen_test(adi_ad9986_device_t *device, adi_ad9986_jesd_link_select_e links, adi_ad9986_jesd_tx_test_data_src_e data_source, adi_ad9986_jesd_tx_test_mode_e test_mode);

/* Use-case parameter tables defined in uc_settings.c.  The Raspberry Pi
 * control-plane app drives a single JESD204C use case; all datapath and link
 * parameters are read directly from index AD9986_UC_INDEX of these tables
 * (rather than being duplicated as local literals in this file). */
#define AD9986_UC_INDEX 1
extern uint8_t                   tx_dac_chan_xbar[][4];
extern int64_t                   tx_main_shift[][4];
extern int64_t                   tx_chan_shift[][8];
extern uint8_t                   tx_interp[][2];
extern uint8_t                   rx_cddc_select[];
extern int64_t                   rx_cddc_shift[][4];
extern uint8_t                   rx_cddc_dcm[][4];
extern uint8_t                   rx_cddc_c2r[][4];
extern uint8_t                   rx_fddc_select[];
extern int64_t                   rx_fddc_shift[][8];
extern uint8_t                   rx_fddc_dcm[][8];
extern uint8_t                   rx_fddc_c2r[8];
extern adi_ad9986_jtx_conv_sel_t jtx_conv_sel[][2];
extern adi_cms_jesd_param_t      jrx_param[];
extern adi_cms_jesd_param_t      jtx_param[][2];

/* Standardized register/status print: offset, symbol, raw value, decoded fields. */
static void app_reg_print_u8(uint32_t off, const char *name, uint8_t val, const char *decode)
{
    printf("  0x%04X  %-32s  val=0x%02X  %s\n",
           (unsigned)off, name, (unsigned)val, decode ? decode : "");
}

static void app_reg_print_u16(uint32_t off, const char *name, uint16_t val, const char *decode)
{
    printf("  0x%04X  %-32s  val=0x%04X  %s\n",
           (unsigned)off, name, (unsigned)val, decode ? decode : "");
}

static void app_reg_print_u32(uint32_t off, const char *name, uint32_t val, const char *decode)
{
    printf("  0x%04X  %-32s  val=0x%08X  %s\n",
           (unsigned)off, name, (unsigned)val, decode ? decode : "");
}

/* Serialises SPI bus access between the PLL monitor thread, PRBS loop, and main menu. */
static pthread_mutex_t g_spi_mtx = PTHREAD_MUTEX_INITIALIZER;

/*============= C O D E ====================*/
static int32_t app_jesd_ip_reg_write(uint32_t addr, uint32_t data)
{
    int32_t err;

    if (err = adi_ads9_reg_set(addr, data), err != API_CMS_ERROR_OK) {
        printf("JESD IP: write addr=0x%04x failed (%d).\n", addr, err);
        return err;
    }
    app_reg_print_u32(addr, "FPGA_JESD_IP", data, "SPI write");
    return API_CMS_ERROR_OK;
}

static int32_t app_jesd_ip_reg_read(uint32_t addr, uint32_t *data)
{
    int32_t err;

    if (data == NULL) {
        return API_CMS_ERROR_NULL_PARAM;
    }
    if (err = adi_ads9_reg_get(addr, data), err != API_CMS_ERROR_OK) {
        printf("JESD IP: read  addr=0x%04x failed (%d).\n", addr, err);
        return err;
    }
    app_reg_print_u32(addr, "FPGA_JESD_IP", *data, "SPI read");
    return API_CMS_ERROR_OK;
}

static int32_t app_jesd_ip_init(void)
{
    int32_t  err;
    uint32_t fpga_ver  = 0;
    uint32_t probe_a   = 0;
    uint32_t probe_b   = 0;

    /* Confirm the JESD IP register bridge is reachable over SPI0 CS0 by
     * reading the FPGA version registers (addr 0x100, 0x101).
     *
     * Note: a write/read scratchpad test (addr 0x11D or similar) is NOT done
     * here.  The Lattice FPGA JESD IP functional registers are clocked by the
     * device clock that arrives from HMC7044 CH_2; they are not writable until
     * that clock is running.  The write/read test is instead performed after
     * app_hmc7044_clk_config() and app_ad9986_clk_config() complete. */
    if (err = adi_ads9_ver_get(&fpga_ver), err != API_CMS_ERROR_OK) {
        printf("JESD IP: version read failed (%d) -- check SPI0 CS0 wiring.\n", err);
        return err;
    }

    /* A successful transfer does not by itself mean the bridge decoded anything.
     * When the FPGA SPI target does not implement the register commands it acts
     * as a one-transaction-delayed loopback, and each read returns the address
     * word that was clocked out rather than register content.  Reading two
     * distinct addresses catches that: if both come back as the address itself,
     * the bridge is echoing and no FPGA configuration write will ever land. */
    if (ads9_axi_reg_read32(0x100, &probe_a) == API_CMS_ERROR_OK &&
        ads9_axi_reg_read32(0x104, &probe_b) == API_CMS_ERROR_OK &&
        probe_a == 0x100u && probe_b == 0x104u) {
        printf("JESD IP: SPI0 CS0 bridge is ECHOING (read of 0x100 returned 0x%08x, "
               "read of 0x104 returned 0x%08x).\n", probe_a, probe_b);
        printf("JESD IP: the FPGA SPI target is not decoding register commands -- "
               "no JESD IP configuration write will take effect.\n");
        printf("JESD IP: check that the RISC-V register-bridge firmware is loaded "
               "and running on the FPGA.\n");
        /* Deliberately not fatal: the AD9986 and HMC7044 buses are independent
         * of this bridge, so bring-up continues and the status dumps stay
         * available for diagnosis.  Only the "bridge OK" claim is withheld. */
        return API_CMS_ERROR_OK;
    }

    printf("JESD IP (FPGA) image v%x -- SPI0 CS0 bridge OK.\n", fpga_ver);
    return API_CMS_ERROR_OK;
}

static int32_t app_jesd_ip_reg_verify(void)
{
    int32_t  err;
    uint32_t readback = 0;

    /* Write/read verify: addr 0x11D is the JRX subclass register.
     * Called after clocks are up so the JESD IP register domain is active. */
    if (err = app_jesd_ip_reg_write(0x40004000, 0x210), err != API_CMS_ERROR_OK) {
        return err;
    }
    if (err = app_jesd_ip_reg_read(0x40004000, &readback), err != API_CMS_ERROR_OK) {
        return err;
    }
    if ((readback & 0xFFFFFFFF) != 0x210) {
        printf("JESD IP: register verify FAILED (wrote 0x01 to 0x011D, read 0x%08x).\n",
               readback);
        return API_CMS_ERROR_TEST_FAILED;
    }
    /* Restore JRX subclass to 0 (default = subclass 0) */
    app_jesd_ip_reg_write(0x40004000, 0x10);
    printf("JESD IP: register write/read verify OK.\n");
    return API_CMS_ERROR_OK;
}

void app_jesd_ip_reg_read_test(void){
	uint32_t readback = 0;

	app_jesd_ip_reg_read(0x40005000, &readback);
	printf("JESD IP: PHY register read value: 0x%x\n", readback);  
	app_jesd_ip_reg_read(0x40006000, &readback);
	printf("JESD IP: RX control register read value: 0x%x\n", readback);
	app_jesd_ip_reg_read(0x40006004, &readback);
	printf("JESD IP: RX status register read value: 0x%x\n", readback);
	app_jesd_ip_reg_read(0x40004000, &readback);
	printf("JESD IP: TX control register read value: 0x%x\n", readback);
	app_jesd_ip_reg_read(0x40004004, &readback);
	printf("JESD IP: TX status register read value: 0x%x\n", readback); 
   
}

static int32_t app_ad9986_identify(adi_ad9986_device_t *dev)
{
    int32_t err;
    uint8_t rev[3] = {0};
    adi_cms_chip_id_t chip_id = {0};

    /* Hard reset: GPIO 25 (AD9986 RESETB) is shared with the Lattice FPGA
     * RESETB on this board, so this also resets the FPGA JESD IP.
     * This is intentional: soft reset does not clear all AD9986 register state
     * (reg 0x001C bit 6 stays set), causing the device_init 8-bit r/w check to
     * fail.  The FPGA recovers from reset because HMC7044 CH_2 continues to
     * supply the device clock during and after the GPIO 25 pulse.  The JESD IP
     * register verify (app_jesd_ip_reg_verify) runs after all clocks are up, by
     * which time the FPGA has fully come out of reset. */
    if (err = adi_ad9986_device_reset(dev, AD9986_HARD_RESET), err != API_CMS_ERROR_OK) {
        printf("AD9986: hard reset failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_device_spi_config(dev), err != API_CMS_ERROR_OK) {
        printf("AD9986: spi config failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_device_init(dev), err != API_CMS_ERROR_OK) {
        printf("AD9986: device init failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_device_api_revision_get(dev, &rev[0], &rev[1], &rev[2]), err != API_CMS_ERROR_OK) {
        return err;
    }
    printf("AD9986 API v%d.%d.%d\n", rev[0], rev[1], rev[2]);
    if (err = adi_ad9986_device_chip_id_get(dev, &chip_id), err != API_CMS_ERROR_OK) {
        printf("AD9986: chip id read failed (%d) -- check SPI0 CS1 / Lattice proxy wiring.\n", err);
        return err;
    }
    {
        char dec[64];
        app_reg_print_u8(0x0003, "REG_CHIP_TYPE", chip_id.chip_type,
                         "[7:0]=BF_CHIP_TYPE");
        snprintf(dec, sizeof(dec), "[15:0]=BF_PROD_ID=0x%04X", chip_id.prod_id);
        app_reg_print_u16(0x0004, "REG_PROD_ID_LSB/MSB", chip_id.prod_id, dec);
        snprintf(dec, sizeof(dec), "[3:0]=BF_CHIP_GRADE=%u", chip_id.dev_revision);
        app_reg_print_u8(0x0006, "REG_CHIP_GRADE", chip_id.dev_revision, dec);
    }
    return API_CMS_ERROR_OK;
}

static int32_t app_hmc7044_identify(adi_hmc7044_device_t *dev)
{
    int32_t err;
    uint8_t rev[3] = {0};
    adi_cms_chip_id_t chip_id = {0};

    if (err = adi_hmc7044_device_api_revision_get(dev, &rev[0], &rev[1], &rev[2]), err != API_CMS_ERROR_OK) {
        return err;
    }
    printf("HMC7044 API v%d.%d.%d\n", rev[0], rev[1], rev[2]);
    if (err = adi_hmc7044_device_chip_id_get(dev, &chip_id), err != API_CMS_ERROR_OK) {
        printf("HMC7044: chip id read failed (%d) -- check SPI1 wiring.\n", err);
        return err;
    }
    {
        char dec[64];
        uint8_t r78 = 0, r79 = 0, r7a = 0;
        adi_hmc7044_device_spi_register_get(dev, 0x0078, &r78);
        adi_hmc7044_device_spi_register_get(dev, 0x0079, &r79);
        adi_hmc7044_device_spi_register_get(dev, 0x007A, &r7a);
        app_reg_print_u8(0x0078, "HMC7044_CHIP_ID_0", r78, "[7:0]=dev_revision");
        snprintf(dec, sizeof(dec), "[7:0]=prod_id[7:0]=0x%02X", r79);
        app_reg_print_u8(0x0079, "HMC7044_CHIP_ID_1", r79, dec);
        snprintf(dec, sizeof(dec), "[7:0]=prod_id[15:8]=0x%02X  prod_id=0x%04X", r7a, chip_id.prod_id);
        app_reg_print_u8(0x007A, "HMC7044_CHIP_ID_2", r7a, dec);
    }

    /* Validate the read-back. A product id of 0x0000 (idle-low data line) or
     * 0xFFFF (floating-high / no driver) means the HMC7044 is not responding:
     * check the SPI1 wiring and the GPO1 -> MISO (SDO) routing required for
     * 4-wire read-back. Abort bring-up here with a clear diagnostic rather
     * than continuing with an unconfigured clock source. */
    if (chip_id.prod_id == 0x0000 || chip_id.prod_id == 0xFFFF) {
        printf("HMC7044: invalid product id 0x%04x -- device not responding "
               "(check SPI1 wiring and GPO1->MISO/SDO routing).\n", chip_id.prod_id);
        return API_CMS_ERROR_TEST_FAILED;
    }
    return API_CMS_ERROR_OK;
}

/* Configure HMC7044 GPIO1 (GPO1) as the SPI serial-data output (SDO).
 *
 * The HMC7044 serial port is natively 3-wire (a single bidirectional SDATA
 * pin). To run 4-wire SPI from the Raspberry Pi 5 (separate MOSI/MISO via
 * spidev), the read-back data must be driven on a dedicated pin. The part
 * provides this by routing the serial readback onto a GPO pin: GPO1 maps to
 * HMC7044 GPIO1 (device pin 31), which is brought out to the Pi MISO through
 * the Lattice FPGA.
 *
 * This is a write-only transaction, so it needs only the 3-wire write path
 * (SCLK / SEN / SDATA-as-input) and must run BEFORE any HMC7044 register read
 * (chip-id, revision, status), because on this 4-wire wiring the read data is
 * returned on GPIO1.
 *
 * GPO1 selection = HMC7044_GPO_SPI_SDATA (drive the SPI readback / SDO),
 * mode = 1 (CMOS push-pull, so the Pi sees clean logic levels),
 * enable = 1. The API writes 0x07 to register 0x0050. */
/* Check HMC7044 PLL1 and PLL2 lock.
 * Must be called after adi_hmc7044_clk_config() programs the PLL dividers
 * AND after app_hmc7044_enable_sdo() enables the 4-wire SPI read path.
 * Returns API_CMS_ERROR_TEST_FAILED if either PLL is not locked; the caller
 * should treat this as fatal once a real clock configuration is in place. */
static int32_t app_hmc7044_pll_lock_check(adi_hmc7044_device_t *dev)
{
    int32_t err;
    uint8_t status = 0;

    if (err = adi_hmc7044_device_pll_lock_status_get(dev, &status), err != API_CMS_ERROR_OK) {
        printf("HMC7044: PLL lock status read failed (%d).\n", err);
        return err;
    }
    {
        char dec[96];
        uint8_t r7c = 0, r7d = 0;
        adi_hmc7044_device_spi_register_get(dev, 0x007C, &r7c);
        adi_hmc7044_device_spi_register_get(dev, 0x007D, &r7d);
        snprintf(dec, sizeof(dec), "[5]=PLL1_lock=%u  API=%s",
                 (r7c >> 5) & 1, (status & HMC7044_PLL1_LOCK_ST) ? "LOCKED" : "NOT LOCKED");
        app_reg_print_u8(0x007C, "HMC7044_PLL1_STATUS", r7c, dec);
        snprintf(dec, sizeof(dec), "[0]=PLL2_lock=%u  API=%s",
                 r7d & 1, (status & HMC7044_PLL2_LOCK_ST) ? "LOCKED" : "NOT LOCKED");
        app_reg_print_u8(0x007D, "HMC7044_PLL2_STATUS", r7d, dec);
    }
    if (!(status & HMC7044_PLL1_AND_PLL2_LOCK_ST)) {
        printf("HMC7044: PLL not fully locked -- call adi_hmc7044_clk_config() first.\n");
        return API_CMS_ERROR_TEST_FAILED;
    }
    printf("HMC7044: PLL locked OK.\n");
    return API_CMS_ERROR_OK;
}

/* Read and print HMC7044 PLL lock status (SPI1 CS0). */
static void app_hmc7044_reg_dump(adi_hmc7044_device_t *dev)
{
    char dec[160];

    printf("\n=== HMC7044 register dump (SPI1 CS0) ===\n");
    {
        uint8_t r7c = 0, r7d = 0, pll_st = 0;
        adi_hmc7044_device_pll_lock_status_get(dev, &pll_st);
        adi_hmc7044_device_spi_register_get(dev, 0x007C, &r7c);
        adi_hmc7044_device_spi_register_get(dev, 0x007D, &r7d);
        snprintf(dec, sizeof(dec), "[5]=PLL1_lock=%u  API=%s",
                 (r7c >> 5) & 1, (pll_st & HMC7044_PLL1_LOCK_ST) ? "LOCKED" : "NOT LOCKED");
        app_reg_print_u8(0x007C, "HMC7044_PLL1_STATUS", r7c, dec);
        snprintf(dec, sizeof(dec), "[0]=PLL2_lock=%u  API=%s",
                 r7d & 1, (pll_st & HMC7044_PLL2_LOCK_ST) ? "LOCKED" : "NOT LOCKED");
        app_reg_print_u8(0x007D, "HMC7044_PLL2_STATUS", r7d, dec);
    }
}

/* Configure the AD9986 on-chip clock PLL.
 *
 * Must be called after adi_ad9986_device_init() and after the HMC7044
 * device clock output (CH_2) has been verified as locked.  The API programs
 * the AD9986 internal PLL to derive the DAC and ADC sample clocks from the
 * device clock input.
 *
 * Clock frequencies are defined in lattice.h (override with -D build flag):
 *   LATTICE_AD9986_DEV_CLK_HZ -- device clock arriving at the AD9986 CLK pin
 *   LATTICE_AD9986_DAC_CLK_HZ -- desired DAC sample rate
 *   LATTICE_AD9986_ADC_CLK_HZ -- desired ADC sample rate */
static int32_t app_ad9986_clk_config(adi_ad9986_device_t *dev)
{
    int32_t err;

    if (err = adi_ad9986_device_clk_config_set(dev,
              LATTICE_AD9986_DAC_CLK_HZ,
              LATTICE_AD9986_ADC_CLK_HZ,
              LATTICE_AD9986_DEV_CLK_HZ), err != API_CMS_ERROR_OK) {
        printf("AD9986: clk_config_set failed (%d) -- check device clock routing.\n", err);
        return err;
    }
    printf("AD9986 clocks configured: ref=%.0f MHz  dac=%.0f MHz  adc=%.0f MHz.\n",
           (double)LATTICE_AD9986_DEV_CLK_HZ / 1.0e6,
           (double)LATTICE_AD9986_DAC_CLK_HZ / 1.0e6,
           (double)LATTICE_AD9986_ADC_CLK_HZ / 1.0e6);
    return API_CMS_ERROR_OK;
}

/* Check AD9986 on-chip clock PLL (fast + slow lock bits).
 * Must be called after adi_ad9986_device_clk_config_set(); status == 0x3
 * means both PLL_LOCK_FAST (bit0) and PLL_LOCK_SLOW (bit1) are set.
 * Per UG-1578 pg.14 this is a mandatory gate before further initialization. */
static int32_t app_ad9986_clk_pll_lock_check(adi_ad9986_device_t *dev)
{
    int32_t err;
    uint8_t status = 0;

    if (err = adi_ad9986_device_clk_pll_lock_status_get(dev, &status), err != API_CMS_ERROR_OK) {
        printf("AD9986: clock PLL lock status read failed (%d).\n", err);
        return err;
    }
    {
        char dec[96];
        snprintf(dec, sizeof(dec), "[0]=BF_PLL_LOCK_SLOW=%s  [1]=BF_PLL_LOCK_FAST=%s",
                 (status & 0x1) ? "LOCKED" : "NOT LOCKED",
                 (status & 0x2) ? "LOCKED" : "NOT LOCKED");
        app_reg_print_u8(0x2008, "REG_CLK_PLL_STATUS", status, dec);
    }
    if (status != 0x3) {
        printf("AD9986: clock PLL not fully locked -- check device clock input.\n");
        return API_CMS_ERROR_TEST_FAILED;
    }
    printf("AD9986: clock PLL locked OK.\n");
    return API_CMS_ERROR_OK;
}

/* Check AD9986 JESD SerDes PLL lock.
 * Must be called after JESD link setup (adi_ads9_config_jesd) and before
 * enabling JESD Rx/Tx links. */
static int32_t __attribute__((unused)) app_ad9986_jesd_pll_lock_check(adi_ad9986_device_t *dev)
{
    int32_t err;
    uint8_t status = 0, r0722 = 0;

    if (err = adi_ad9986_jesd_pll_lock_status_get(dev, &status), err != API_CMS_ERROR_OK) {
        printf("AD9986: JESD PLL lock status read failed (%d).\n", err);
        return err;
    }
    adi_ad9986_hal_reg_get(dev, 0x0722, &r0722);
    {
        char dec[64];
        snprintf(dec, sizeof(dec), "[3]=BF_RFPLLLOCK_LCPLL_RS=%u  %s",
                 (r0722 >> 3) & 1, status ? "LOCKED" : "NOT LOCKED");
        app_reg_print_u8(0x0722, "REG_PLL_STATUS_LCPLL", r0722, dec);
    }
    if (!status) {
        printf("AD9986: JESD SerDes PLL not locked -- check lane rate / JESD clock.\n");
        return API_CMS_ERROR_TEST_FAILED;
    }
    printf("AD9986: JESD PLL locked OK.\n");
    return API_CMS_ERROR_OK;
}

/* Query and print JESD204 link status for both JRX (DAC path) and JTX (ADC path).
 * Can be called any time after clock bring-up; results are only meaningful once
 * the JESD link has been configured and enabled.
 *
 * Output is organised in two sections:
 *   1. AD9986 register dump (SPI0 CS1), ascending address, 204C-focused
 *   2. HMC7044 register dump (SPI1 CS0)
 */
static int32_t app_ad9986_link_status_check(adi_ad9986_device_t *dev,
                                            adi_hmc7044_device_t *hmc)
{
    int32_t  err;
    char     dec[160];

    uint8_t  jesd_pll = 0;
    uint8_t  r0722 = 0;
    uint8_t  r01FE = 0, r01FF = 0;
    uint8_t  r0289 = 0;
    uint8_t  r04A0 = 0;
    uint8_t  r04A9 = 0, r04AA = 0, r04AB = 0, r04AC = 0;
    uint8_t  r04AD = 0, r04AE = 0, r04AF = 0, r04B0 = 0;
    uint8_t  r055E = 0, r05BB = 0;
    uint8_t  r0636 = 0;
    uint8_t  r063D = 0, r063E = 0, r063F = 0, r0640 = 0;
    uint8_t  r0641 = 0, r0642 = 0, r0643 = 0, r0644 = 0;
    uint8_t  r0667 = 0, r0668 = 0;
    uint8_t  r0701 = 0, r0711 = 0, r0713 = 0;
    uint8_t  r00B8 = 0;
    uint16_t sysref_phase = 0;
    uint8_t  sysref_sync_done = 0;
    uint8_t  jrx_cfg_valid = 0;
    uint8_t  irq_st = 0;

    /* ------------------------------------------------------------------ */
    /* Register reads (API + direct)                                         */
    /* ------------------------------------------------------------------ */
    if (err = adi_ad9986_jesd_pll_lock_status_get(dev, &jesd_pll), err != API_CMS_ERROR_OK) {
        printf("AD9986: JESD SerDes PLL read failed (%d).\n", err);
        return err;
    }

    adi_ad9986_hal_reg_get(dev, 0x01FE, &r01FE);
    adi_ad9986_hal_reg_get(dev, 0x01FF, &r01FF);
    adi_ad9986_hal_reg_get(dev, 0x00B8, &r00B8);
    adi_ad9986_jesd_sysref_monitor_phase_get(dev, &sysref_phase);
    adi_ad9986_jesd_sysref_oneshot_sync_done_get(dev, &sysref_sync_done);

    adi_ad9986_jesd_tx_link_select_set(dev, AD9986_LINK_0);
    adi_ad9986_hal_reg_get(dev, 0x0289, &r0289);

    adi_ad9986_jesd_rx_link_select_set(dev, AD9986_LINK_0);
    adi_ad9986_hal_reg_get(dev, 0x04A0, &r04A0);
    adi_ad9986_hal_reg_get(dev, 0x04A9, &r04A9);
    adi_ad9986_hal_reg_get(dev, 0x04AA, &r04AA);
    adi_ad9986_hal_reg_get(dev, 0x04AB, &r04AB);
    adi_ad9986_hal_reg_get(dev, 0x04AC, &r04AC);
    adi_ad9986_hal_reg_get(dev, 0x04AD, &r04AD);
    adi_ad9986_hal_reg_get(dev, 0x04AE, &r04AE);
    adi_ad9986_hal_reg_get(dev, 0x04AF, &r04AF);
    adi_ad9986_hal_reg_get(dev, 0x04B0, &r04B0);
    adi_ad9986_hal_reg_get(dev, 0x055E, &r055E);
    adi_ad9986_hal_reg_get(dev, 0x05BB, &r05BB);

    adi_ad9986_jesd_rx_config_status_get(dev, &jrx_cfg_valid);
    adi_ad9986_jesd_rx_204c_crc_irq_status_get(dev, AD9986_LINK_0, &irq_st);
    adi_ad9986_jesd_rx_204c_crc_irq_clr(dev, AD9986_LINK_0);
    adi_ad9986_jesd_rx_204c_mb_irq_status_get(dev, AD9986_LINK_0, &irq_st);
    adi_ad9986_jesd_rx_204c_mb_irq_clr(dev, AD9986_LINK_0);
    adi_ad9986_jesd_rx_204c_sh_irq_status_get(dev, AD9986_LINK_0, &irq_st);
    adi_ad9986_jesd_rx_204c_sh_irq_clr(dev, AD9986_LINK_0);

    adi_ad9986_jesd_tx_link_select_set(dev, AD9986_LINK_0);
    adi_ad9986_hal_reg_get(dev, 0x0636, &r0636);
    adi_ad9986_hal_reg_get(dev, 0x063D, &r063D);
    adi_ad9986_hal_reg_get(dev, 0x063E, &r063E);
    adi_ad9986_hal_reg_get(dev, 0x063F, &r063F);
    adi_ad9986_hal_reg_get(dev, 0x0640, &r0640);
    adi_ad9986_hal_reg_get(dev, 0x0641, &r0641);
    adi_ad9986_hal_reg_get(dev, 0x0642, &r0642);
    adi_ad9986_hal_reg_get(dev, 0x0643, &r0643);
    adi_ad9986_hal_reg_get(dev, 0x0644, &r0644);
    adi_ad9986_hal_reg_get(dev, 0x0667, &r0667);
    adi_ad9986_hal_reg_get(dev, 0x0668, &r0668);

    adi_ad9986_hal_reg_get(dev, 0x0701, &r0701);
    adi_ad9986_hal_reg_get(dev, 0x0711, &r0711);
    adi_ad9986_hal_reg_get(dev, 0x0713, &r0713);
    adi_ad9986_hal_reg_get(dev, 0x0722, &r0722);

    /* ------------------------------------------------------------------ */
    /* Section 1 — AD9986 register dump (SPI0 CS1), ascending address      */
    /* ------------------------------------------------------------------ */
    printf("\n=== AD9986 register dump (SPI0 CS1) ===\n");

    printf("  [SYSREF monitor — not paged]\n");
    snprintf(dec, sizeof(dec),
             "[12:0]=BF_SYSREF_PHASE=0x%04X  (API read of 0x00B5/0x00B6)",
             sysref_phase);
    app_reg_print_u16(0x00B5, "REG_SYSREF_PHASE0/1", sysref_phase, dec);
    snprintf(dec, sizeof(dec), "[4]=BF_ONESHOT_SYNC_DONE=%u", sysref_sync_done);
    app_reg_print_u8(0x00B8, "REG_SYSREF_MODE", r00B8, dec);

    printf("  [JESD mode / datapath — not paged]\n");
    snprintf(dec, sizeof(dec),
             "[5:0]=BF_TX_JESD_MODE=%u  [7]=BF_MODE_NOT_IN_TABLE=%u  (0=OK, 1=mismatch)",
             r01FE & 0x3F, jrx_cfg_valid);
    app_reg_print_u8(0x01FE, "REG_JESD_MODE", r01FE, dec);
    snprintf(dec, sizeof(dec),
             "[7:4]=BF_FINE_INTERP_SEL=%u  [3:0]=BF_COARSE_INTERP_SEL=%u  product=%u",
             (r01FF >> 4) & 0x0F, r01FF & 0x0F,
             ((r01FF >> 4) & 0x0F) * (r01FF & 0x0F));
    app_reg_print_u8(0x01FF, "REG_INTRP_MODE", r01FF, dec);

    printf("  [JTX framer — paged link0]\n");
    snprintf(dec, sizeof(dec),
             "[7:0]=BF_CHIP_DECIMATION_RATIO=%u  (expect DCM=8 for UC1)", r0289 & 0xFF);
    app_reg_print_u8(0x0289, "REG_CHIP_DECIMATION_RATIO", r0289, dec);

    printf("  [JRX deframer — paged link0]\n");
    snprintf(dec, sizeof(dec), "[0]=%u  [2]=BF_JRX_TPL_SYSREF_RCVD=%u",
             r04A0 & 1, (r04A0 >> 2) & 1);
    app_reg_print_u8(0x04A0, "REG_JRX_TPL_0", r04A0, dec);
    snprintf(dec, sizeof(dec),
             "[4:0]=BF_JRX_L_CFG=%u(L=%u)  [7]=BF_JRX_DSCR_CFG=%u  (expect L=1)",
             r04A9 & 0x1F, (r04A9 & 0x1F) + 1, (r04A9 >> 7) & 1);
    app_reg_print_u8(0x04A9, "REG_JRX_L0_3", r04A9, dec);
    snprintf(dec, sizeof(dec), "[7:0]=BF_JRX_F_CFG=%u(F=%u)  (expect F=4)",
             r04AA & 0xFF, (r04AA & 0xFF) + 1);
    app_reg_print_u8(0x04AA, "REG_JRX_L0_4", r04AA, dec);
    snprintf(dec, sizeof(dec), "[7:0]=BF_JRX_K_CFG=%u(K=%u)  (expect K=64)",
             r04AB & 0xFF, (r04AB & 0xFF) + 1);
    app_reg_print_u8(0x04AB, "REG_JRX_L0_5", r04AB, dec);
    snprintf(dec, sizeof(dec), "[7:0]=BF_JRX_M_CFG=%u(M=%u)  (expect M=2)",
             r04AC & 0xFF, (r04AC & 0xFF) + 1);
    app_reg_print_u8(0x04AC, "REG_JRX_L0_6", r04AC, dec);
    snprintf(dec, sizeof(dec), "[4:0]=BF_JRX_N_CFG=%u(N=%u)  (expect N=16)",
             r04AD & 0x1F, (r04AD & 0x1F) + 1);
    app_reg_print_u8(0x04AD, "REG_JRX_L0_7", r04AD, dec);
    snprintf(dec, sizeof(dec),
             "[4:0]=BF_JRX_NP_CFG=%u(Np=%u)  [7:5]=BF_JRX_SUBCLASSV_CFG=%u  (expect Np=16 sub=1)",
             r04AE & 0x1F, (r04AE & 0x1F) + 1, (r04AE >> 5) & 7);
    app_reg_print_u8(0x04AE, "REG_JRX_L0_8", r04AE, dec);
    snprintf(dec, sizeof(dec),
             "[4:0]=BF_JRX_S_CFG=%u(S=%u)  [7:5]=BF_JRX_JESDV_CFG=%u  (expect S=1 jesdv=2)",
             r04AF & 0x1F, (r04AF & 0x1F) + 1, (r04AF >> 5) & 7);
    app_reg_print_u8(0x04AF, "REG_JRX_L0_9", r04AF, dec);
    snprintf(dec, sizeof(dec), "[7]=BF_JRX_HD_CFG=%u  (expect HD=0)", (r04B0 >> 7) & 1);
    app_reg_print_u8(0x04B0, "REG_JRX_L0_10", r04B0, dec);

    printf("  [JRX deframer — 204C link state, paged link0]\n");
    snprintf(dec, sizeof(dec),
             "[7]=BF_JRX_DL_204C_ENABLE=%u  [6:4]=BF_JRX_DL_204C_STATE=%u  %s  (expect en=1)",
             (r055E >> 7) & 1, (r055E >> 4) & 7,
             ((r055E >> 4) & 7) == 6 ? "(link up)" : "(link down)");
    app_reg_print_u8(0x055E, "REG_JRX_DL_204C_0", r055E, dec);
    {
        uint32_t K = (r04AB & 0xFF) + 1, F = (r04AA & 0xFF) + 1;
        uint32_t E = (K * F) / 256;
        printf("  (derived)   E (JRX, K*F/256)            K=%u F=%u => E=%u  (expect E=1)\n",
               K, F, E < 1 ? 1 : E);
    }

    printf("  [JRX deframer — 204C error IRQ, paged link0]\n");
    snprintf(dec, sizeof(dec),
             "[3]=BF_JRX_204C_CRC_IRQ=%u  [4]=BF_JRX_204C_MB_IRQ=%u  [5]=BF_JRX_204C_SH_IRQ=%u"
             "  (latched, cleared after read)",
             (r05BB >> 3) & 1, (r05BB >> 4) & 1, (r05BB >> 5) & 1);
    app_reg_print_u8(0x05BB, "REG_JRX_204C_IRQ", r05BB, dec);

    printf("  [JTX framer — paged link0, continued]\n");
    snprintf(dec, sizeof(dec),
             "[0]=BF_JTX_TPL_INVALID_CFG=%u  [1]=BF_JTX_TPL_SYSREF_RCVD=%u"
             "  [2]=BF_JTX_TPL_SYSREF_PHASE_ERR=%u  [5]=BF_JTX_TPL_SYSREF_MASK=%u",
             r0636 & 1, (r0636 >> 1) & 1, (r0636 >> 2) & 1, (r0636 >> 5) & 1);
    app_reg_print_u8(0x0636, "REG_JTX_TPL_6", r0636, dec);
    snprintf(dec, sizeof(dec),
             "[4:0]=BF_JTX_L_CFG=%u(L=%u)  [7]=BF_JTX_SCR_CFG=%u  (expect L=1)",
             r063D & 0x1F, (r063D & 0x1F) + 1, (r063D >> 7) & 1);
    app_reg_print_u8(0x063D, "REG_JTX_L0_3", r063D, dec);
    snprintf(dec, sizeof(dec), "[7:0]=BF_JTX_F_CFG=%u(F=%u)  (expect F=4)",
             r063E & 0xFF, (r063E & 0xFF) + 1);
    app_reg_print_u8(0x063E, "REG_JTX_L0_4", r063E, dec);
    snprintf(dec, sizeof(dec), "[7:0]=BF_JTX_K_CFG=%u(K=%u)  (expect K=64)",
             r063F & 0xFF, (r063F & 0xFF) + 1);
    app_reg_print_u8(0x063F, "REG_JTX_L0_5", r063F, dec);
    snprintf(dec, sizeof(dec), "[7:0]=BF_JTX_M_CFG=%u(M=%u)  (expect M=2)",
             r0640 & 0xFF, (r0640 & 0xFF) + 1);
    app_reg_print_u8(0x0640, "REG_JTX_L0_6", r0640, dec);
    snprintf(dec, sizeof(dec),
             "[4:0]=BF_JTX_N_CFG=%u(N=%u)  [7:6]=BF_JTX_CS_CFG=%u  (expect N=16)",
             r0641 & 0x1F, (r0641 & 0x1F) + 1, (r0641 >> 6) & 3);
    app_reg_print_u8(0x0641, "REG_JTX_L0_7", r0641, dec);
    snprintf(dec, sizeof(dec),
             "[4:0]=BF_JTX_NP_CFG=%u(Np=%u)  [7:5]=BF_JTX_SUBCLASSV_CFG=%u  (expect Np=16 sub=1)",
             r0642 & 0x1F, (r0642 & 0x1F) + 1, (r0642 >> 5) & 7);
    app_reg_print_u8(0x0642, "REG_JTX_L0_8", r0642, dec);
    snprintf(dec, sizeof(dec),
             "[4:0]=BF_JTX_S_CFG=%u(S=%u)  [7:5]=BF_JTX_JESDV_CFG=%u  (expect S=1 jesdv=2)",
             r0643 & 0x1F, (r0643 & 0x1F) + 1, (r0643 >> 5) & 7);
    app_reg_print_u8(0x0643, "REG_JTX_L0_9", r0643, dec);
    snprintf(dec, sizeof(dec), "[7]=BF_JTX_HD_CFG=%u  (expect HD=0)", (r0644 >> 7) & 1);
    app_reg_print_u8(0x0644, "REG_JTX_L0_10", r0644, dec);
    snprintf(dec, sizeof(dec), "[3]=BF_JTX_DL_204C_SYSREF_RCVD=%u", (r0667 >> 3) & 1);
    app_reg_print_u8(0x0667, "REG_JTX_DL_204C_0", r0667, dec);
    snprintf(dec, sizeof(dec),
             "[7:0]=BF_JTX_E_CFG=%u(E=%u)  (expect E=1 for UC1)",
             r0668 & 0xFF, (r0668 & 0xFF) + 1);
    app_reg_print_u8(0x0668, "REG_JTX_DL_204C_1", r0668, dec);

    printf("  [JTX framer — not paged]\n");
    snprintf(dec, sizeof(dec), "[7]=BF_JTX_PLL_LOCKED=%u  %s",
             (r0701 >> 7) & 1, ((r0701 >> 7) & 1) ? "LOCKED" : "NOT LOCKED");
    app_reg_print_u8(0x0701, "REG_PLL_STATUS", r0701, dec);
    snprintf(dec, sizeof(dec), "[0]=BF_JTX_INVALID_MODE=%u  %s",
             r0711 & 1, (r0711 & 1) ? "YES (error)" : "NO (ok)");
    app_reg_print_u8(0x0711, "REG_QC_MODE_STATUS", r0711, dec);
    snprintf(dec, sizeof(dec), "[0]=BF_JTX_PHASE_ESTABLISHED=%u  %s",
             r0713 & 1, (r0713 & 1) ? "YES" : "NO");
    app_reg_print_u8(0x0713, "REG_PHASE_ESTABLISH_STATUS", r0713, dec);

    printf("  [SerDes PLL — not paged]\n");
    snprintf(dec, sizeof(dec), "[3]=BF_RFPLLLOCK_LCPLL_RS=%u  %s",
             (r0722 >> 3) & 1, jesd_pll ? "LOCKED" : "NOT LOCKED");
    app_reg_print_u8(0x0722, "REG_PLL_STATUS_LCPLL", r0722, dec);

    {
        uint64_t jrx_rate = 0, jtx_rate = 0;
        if (adi_ad9986_jesd_rx_bit_rate_get(dev, &jrx_rate) == API_CMS_ERROR_OK)
            printf("  (computed)  JRX lane rate  : %llu bps (%.6f Gbps)  [uses 0x01FF + dac_freq]\n",
                   (unsigned long long)jrx_rate, (double)jrx_rate / 1.0e9);
        else
            printf("  (computed)  JRX lane rate  : read failed\n");
        if (adi_ad9986_jesd_tx_bit_rate_get(dev, AD9986_LINK_0, &jtx_rate) == API_CMS_ERROR_OK)
            printf("  (computed)  JTX lane rate  : %llu bps (%.6f Gbps)  [uses 0x0289 + dac_freq]\n",
                   (unsigned long long)jtx_rate, (double)jtx_rate / 1.0e9);
        else
            printf("  (computed)  JTX lane rate  : read failed\n");
    }

    /* ------------------------------------------------------------------ */
    /* Section 2 — HMC7044 register dump (SPI1 CS0)                        */
    /* ------------------------------------------------------------------ */
    app_hmc7044_reg_dump(hmc);

    return API_CMS_ERROR_OK;
}

/* Read the four JRX 204C error/IRQ fields once, print them, clear IRQ latches.
 * Returns 0 if clean, -1 if any error detected. */
static int32_t app_ad9986_irq_once(adi_ad9986_device_t *dev)
{
    uint8_t cfg_valid=0, irq_st=0;
    uint8_t r01FE=0, r05BB=0;
    char dec[128];

    adi_ad9986_hal_reg_get(dev, 0x01FE, &r01FE);
    adi_ad9986_hal_reg_get(dev, 0x05BB, &r05BB);
    adi_ad9986_jesd_rx_config_status_get(dev, &cfg_valid);
    adi_ad9986_jesd_rx_204c_crc_irq_status_get(dev, AD9986_LINK_0, &irq_st);
    adi_ad9986_jesd_rx_204c_crc_irq_clr(dev, AD9986_LINK_0);
    adi_ad9986_jesd_rx_204c_mb_irq_status_get(dev, AD9986_LINK_0, &irq_st);
    adi_ad9986_jesd_rx_204c_mb_irq_clr(dev, AD9986_LINK_0);
    adi_ad9986_jesd_rx_204c_sh_irq_status_get(dev, AD9986_LINK_0, &irq_st);
    adi_ad9986_jesd_rx_204c_sh_irq_clr(dev, AD9986_LINK_0);
    printf("AD9986 JRX 204C error IRQ status:\n");
    snprintf(dec, sizeof(dec),
             "[5:0]=BF_TX_JESD_MODE=%u  [7]=BF_MODE_NOT_IN_TABLE=%u  (0=OK, 1=mismatch)",
             r01FE & 0x3F, cfg_valid);
    app_reg_print_u8(0x01FE, "REG_JESD_MODE", r01FE, dec);
    snprintf(dec, sizeof(dec),
             "[3]=BF_JRX_204C_CRC_IRQ=%u  [4]=BF_JRX_204C_MB_IRQ=%u  [5]=BF_JRX_204C_SH_IRQ=%u"
             "  (latched, cleared after read)",
             (r05BB >> 3) & 1, (r05BB >> 4) & 1, (r05BB >> 5) & 1);
    app_reg_print_u8(0x05BB, "REG_JRX_204C_IRQ", r05BB, dec);
    return (cfg_valid == 1 || (r05BB & 0x38)) ? -1 : 0;
}

/* Run one 1-second JRX sample PRBS window under SPI lock. */
static int32_t app_ad9986_jrx_sample_prbs_once(adi_ad9986_device_t *dev,
    uint8_t *error_flag, uint32_t *error_count_i, uint32_t *error_count_q)
{
    int32_t err;

    pthread_mutex_lock(&g_spi_mtx);
    err = adi_ad9986_jesd_rx_sample_prbs_test(dev, PRBS7, 0, 1);
    if (err == API_CMS_ERROR_OK)
        err = adi_ad9986_jesd_rx_sample_prbs_test_result_get(dev, error_flag, error_count_i, error_count_q);
    pthread_mutex_unlock(&g_spi_mtx);
    return err;
}

/* AD9986 JRX transport-layer PRBS7 checker (FPGA→AD9986 direction).
 * FPGA TX already drives transport-layer PRBS7 via ad_pngen_new1 in top.sv
 * (hardwired to jesd_tx_tl_in_tdata — no software setup required).
 * Loops the AD9986 sample PRBS checker in 1-second windows.
 * Startup windows are cleared silently until error_flag reaches 0; only then
 * are results printed and non-zero errors treated as real link failures. */
static void app_ad9986_jrx_sample_prbs(adi_ad9986_device_t *dev)
{
    int32_t  err;
    uint8_t  error_flag;
    uint32_t error_count_i, error_count_q;
    uint32_t iter = 0;

    printf("AD9986 JRX sample PRBS7 checker (FPGA→AD9986, transport layer).\n");
    printf("Press Ctrl+C to stop.\n");

    /* Clear startup false errors: each test window pulses CLR_ERRORS internally.
     * Do not print anything until the first window reads back all zeros. */
    printf("Clearing PRBS errors to baseline 0 (not printing yet)...\n");
    for (;;) {
        err = app_ad9986_jrx_sample_prbs_once(dev, &error_flag, &error_count_i, &error_count_q);
        if (err != API_CMS_ERROR_OK) {
            printf("JRX sample PRBS: clear phase failed (%d).\n", err);
            return;
        }
        if (error_flag == 0)
            break;
    }
    printf("PRBS baseline cleared — monitoring started.\n");

    while (1) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char dec[160];
        char dec32[96];

        iter++;

        err = app_ad9986_jrx_sample_prbs_once(dev, &error_flag, &error_count_i, &error_count_q);
        if (err != API_CMS_ERROR_OK) {
            printf("JRX sample PRBS: test/result failed (%d).\n", err);
            break;
        }
        printf("[%02d:%02d:%02d] iter=%-4u\n",
               t->tm_hour, t->tm_min, t->tm_sec, iter);
        snprintf(dec, sizeof(dec),
                 "[0]=BF_PRBS_INVALID_DATA_FLAG_I=%u  [1]=BF_PRBS_INVALID_DATA_FLAG_Q=%u"
                 "  [2]=BF_PRBS_ERROR_FLAG_I=%u  [3]=BF_PRBS_ERROR_FLAG_Q=%u  %s",
                 error_flag & 1, (error_flag >> 1) & 1,
                 (error_flag >> 2) & 1, (error_flag >> 3) & 1,
                 error_flag ? "ERROR" : "OK");
        app_reg_print_u8(0x2063, "REG_SAMPLE_PRBS_STATUS0", error_flag, dec);
        snprintf(dec32, sizeof(dec32), "[23:0]=BF_ERROR_COUNT_I=%u", error_count_i);
        app_reg_print_u32(0x2064, "REG_SAMPLE_PRBS_STATUS1", error_count_i, dec32);
        snprintf(dec32, sizeof(dec32), "[23:0]=BF_ERROR_COUNT_Q=%u", error_count_q);
        app_reg_print_u32(0x2067, "REG_SAMPLE_PRBS_STATUS3", error_count_q, dec32);
        if (error_flag)
            break;
    }
}

/* Read HMC7044 CH_13 output control registers and report clock output status.
 *
 * CH_13 is a continuous 7.68 MHz clock output (VCO 2703.36 MHz ÷ divider 352).
 * "Up and running" is confirmed by:
 *   - CTRL_0 bit[0] = 1  (channel enable)
 *   - CTRL_0 bits[3:2] = 0  (startup_mode = 0 = continuous clock)
 *   - Divider (CTRL_1 + CTRL_2) = 352 = 0x160  (correct for 7.68 MHz output)
 *   - CTRL_8 bits[7:6] = 0  (force_mute = 0, output active)
 *   - HMC7044 PLLs locked  (prerequisite — checked separately by app_hmc7044_pll_lock_check)
 *
 * CH_13 register base = 0x00C8 + 13 × 0x0A = 0x014A. */
static int32_t app_hmc7044_ch13_status(adi_hmc7044_device_t *dev)
{
    int32_t err;
    uint8_t ctrl0 = 0, ctrl1 = 0, ctrl2 = 0, ctrl8 = 0;
    uint16_t divider;

    /* CH_13 CTRL_0: enable, startup_mode, sync_en, high_perform_en */
    if (err = adi_hmc7044_device_spi_register_get(dev, 0x014A, &ctrl0), err != API_CMS_ERROR_OK) {
        printf("HMC7044 CH_13: SPI read 0x014A failed (%d).\n", err);
        return err;
    }
    /* CH_13 CTRL_1/CTRL_2: channel divider LSB/MSB */
    if (err = adi_hmc7044_device_spi_register_get(dev, 0x014B, &ctrl1), err != API_CMS_ERROR_OK) {
        printf("HMC7044 CH_13: SPI read 0x014B failed (%d).\n", err);
        return err;
    }
    if (err = adi_hmc7044_device_spi_register_get(dev, 0x014C, &ctrl2), err != API_CMS_ERROR_OK) {
        printf("HMC7044 CH_13: SPI read 0x014C failed (%d).\n", err);
        return err;
    }
    /* CH_13 CTRL_8: driver config (force_mute, driver_mode, driver_impedance) */
    if (err = adi_hmc7044_device_spi_register_get(dev, 0x0152, &ctrl8), err != API_CMS_ERROR_OK) {
        printf("HMC7044 CH_13: SPI read 0x0152 failed (%d).\n", err);
        return err;
    }

    divider = (uint16_t)ctrl1 | ((uint16_t)(ctrl2 & 0x0F) << 8);

    {
        char dec[160];
        printf("HMC7044 CH_13 clock output status:\n");
        snprintf(dec, sizeof(dec),
                 "[0]=CH_enable=%u  [3:2]=CH_startup_mode=%u  [5]=CH_slip_en=%u"
                 "  [6]=CH_sync_en=%u  [7]=CH_high_perform_en=%u",
                 ctrl0 & 1, (ctrl0 >> 2) & 3, (ctrl0 >> 5) & 1,
                 (ctrl0 >> 6) & 1, (ctrl0 >> 7) & 1);
        app_reg_print_u8(0x014A, "CH_13_CTRL_0", ctrl0, dec);
        snprintf(dec, sizeof(dec), "[7:0]=CH_divider[7:0]=%u", ctrl1);
        app_reg_print_u8(0x014B, "CH_13_CTRL_1", ctrl1, dec);
        snprintf(dec, sizeof(dec), "[3:0]=CH_divider[11:8]=%u", ctrl2 & 0x0F);
        app_reg_print_u8(0x014C, "CH_13_CTRL_2", ctrl2, dec);
        printf("  (derived)   CH_13 divider           = %u  (expected 352 = 0x160)\n", divider);
        snprintf(dec, sizeof(dec),
                 "[7:6]=CH_force_mute=%u  [4:3]=CH_driver_mode=%u  [1:0]=CH_impedance=%u",
                 (ctrl8 >> 6) & 3, (ctrl8 >> 3) & 3, ctrl8 & 3);
        app_reg_print_u8(0x0152, "CH_13_CTRL_8", ctrl8, dec);
    }

    /* Summary */
    int ch_enabled     = (ctrl0 & 1);
    int startup_cont   = (((ctrl0 >> 2) & 3) == 0); /* 0 = continuous clock */
    int divider_ok     = (divider == 352);
    int not_muted      = (((ctrl8 >> 6) & 3) == 0);

    printf("HMC7044 CH_13: %s\n",
           (ch_enabled && startup_cont && divider_ok && not_muted)
           ? "UP AND RUNNING (enabled, continuous clock, divider=352, not muted)"
           : "NOT RUNNING (see detail above)");

    return API_CMS_ERROR_OK;
}

/* Unmute HMC7044 CH_13 SYSREF output.
 *
 * adi_hmc7044_clk_config() leaves CTRL_8 bits[7:6] = 0b10 (force_mute = always muted)
 * on SYSREF channels.  Writing 0x00 to reg 0x0152 clears force_mute so the SYSREF
 * signal reaches the FPGA JESD IP.  Call after app_hmc7044_clk_config(). */
static int32_t app_hmc7044_ch13_unmute(adi_hmc7044_device_t *dev)
{
    int32_t err;
    uint8_t ctrl8 = 0;

    /* Read-modify-write: clear only bits[7:6] (force_mute); preserve
     * driver_mode[4:3] and driver_impedance[1:0] which were set by the API. */
    if (err = adi_hmc7044_device_spi_register_get(dev, 0x0152, &ctrl8), err != API_CMS_ERROR_OK) {
        printf("HMC7044 CH_13: SPI read 0x0152 failed (%d).\n", err);
        return err;
    }
    {
        char dec[96];
        snprintf(dec, sizeof(dec), "[7:6]=CH_force_mute=%u  (before unmute)", (ctrl8 >> 6) & 3);
        app_reg_print_u8(0x0152, "CH_13_CTRL_8", ctrl8, dec);
    }

    ctrl8 &= 0x3F; /* clear force_mute bits[7:6] */

    if (err = adi_hmc7044_device_spi_register_set(dev, 0x0152, ctrl8), err != API_CMS_ERROR_OK) {
        printf("HMC7044 CH_13: SPI write 0x0152 failed (%d).\n", err);
        return err;
    }

    /* Readback to confirm */
    ctrl8 = 0;
    if (err = adi_hmc7044_device_spi_register_get(dev, 0x0152, &ctrl8), err != API_CMS_ERROR_OK) {
        printf("HMC7044 CH_13: SPI read 0x0152 failed (%d).\n", err);
        return err;
    }
    {
        char dec[96];
        snprintf(dec, sizeof(dec), "[7:6]=CH_force_mute=%u  %s",
                 (ctrl8 >> 6) & 3,
                 ((ctrl8 >> 6) & 3) == 0 ? "(not muted -- output active)"
                                        : "(still muted -- check SPI write path)");
        app_reg_print_u8(0x0152, "CH_13_CTRL_8", ctrl8, dec);
    }
    return API_CMS_ERROR_OK;
}

static int32_t app_hmc7044_enable_sdo(adi_hmc7044_device_t *dev)
{
    int32_t err;

    if (err = adi_hmc7044_gpo_config_set(dev,
                                         0,                     /* GPO1 -> GPIO1 (device pin 31) */
                                         HMC7044_GPO_SPI_SDATA, /* output SPI readback (SDO)      */
                                         1,                     /* CMOS push-pull driver          */
                                         1),                    /* enable GPO1                    */
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: GPIO1 SDO config failed (%d) -- check SPI1 write path.\n", err);
        return err;
    }
    printf("HMC7044 GPIO1 configured as SDO (4-wire read-back enabled).\n");
    return API_CMS_ERROR_OK;
}

/* Configure the HMC7044 PLL and output clocks.
 *
 * Clock configuration sequence (adapted from the ADS9 reference app):
 *   1. Configure CLKIN0/1 input buffers (100Ω differential, AC-coupled).
 *   2. Select the internal 3-GHz VCO.
 *   3. Program PLL1/PLL2 dividers and output channel dividers via
 *      adi_hmc7044_clk_config().
 *   4. Enable SYSREF and apply high-performance output mode.
 *   5. Commit the register shadow to the device (reg_update).
 *   6. Phase-align outputs (reseed).
 *   7. Wait 100 ms for the PLL to settle.
 *
 * Clock parameters are in lattice.h (override via -D build flag):
 *   LATTICE_HMC7044_REF_CLK_HZ -- crystal / oscillator on CLKIN0
 *   LATTICE_AD9986_DEV_CLK_HZ  -- AD9986 device clock on CH_2
 *
 * Extend hmc_out_ch and hmc_out_hz[] for additional board outputs
 * (FPGA reference, SYSREF, etc.) once the board routing is known. */
static int32_t app_hmc7044_clk_config(adi_hmc7044_device_t *dev)
{
    int32_t err;
    int     i;

    /* Reference oscillator on CLKIN0; fVCXO = reference for crystal input */
    const uint64_t ref_hz = LATTICE_HMC7044_REF_CLK_HZ;

    /* Clock input priority: CLK_IN_0 = highest, others unused */
    uint8_t pri[4] = {1, 0, 2, 3};

    /* CH_2 → AD9986 device clock (board trace from HMC7044 CH_2 to AD9986 CLK input).
     * CH_3  → SYSREF 7.68 MHz.  CH_13 → 7.68 MHz continuous clock (startup_mode=0).
     * CH_6 and CH_12 are additional 122.88 MHz outputs for FPGA / JESD reference.
     * NOTE: the output-disable loop from the ADS9 reference app is intentionally
     * omitted here.  On this Lattice board the FPGA JESD IP uses one of the
     * other HMC7044 outputs as its reference clock; disabling all outputs before
     * clk_config removes that clock and breaks the SPI register bridge on the
     * next run.  Outputs not listed in hmc_out_ch keep their init-table values. */
    const uint16_t hmc_out_ch = HMC7044_OP_CH_2  | HMC7044_OP_CH_3  |
                                 HMC7044_OP_CH_6  | HMC7044_OP_CH_12 |
                                 HMC7044_OP_CH_13;

    uint64_t hmc_out_hz[14] = {
        0, 0,
        LATTICE_AD9986_DEV_CLK_HZ,  /* CH_2  → AD9986 device clock (122.88 MHz) */
        7680000ULL,                 /* CH_3  → SYSREF 7.68 MHz */
        0, 0,
        122880000ULL,               /* CH_6  → 122.88 MHz */
        0, 0, 0, 0, 0,
        122880000ULL,               /* CH_12 → 122.88 MHz (JESD204 reference) */
        7680000ULL                  /* CH_13 → 7.68 MHz continuous clock */
    };

    /* Step 1: CLKIN0 and CLKIN1 -- 100Ω differential termination, AC-coupled */
    if (err = adi_hmc7044_input_reference_set(dev, 0,
              IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, 1),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: CLKIN0 input buffer config failed (%d).\n", err);
        return err;
    }
    if (err = adi_hmc7044_input_reference_set(dev, 1,
              IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, 1),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: CLKIN1 input buffer config failed (%d).\n", err);
        return err;
    }
    if (err = adi_hmc7044_input_reference_los_config_set(dev, 7, 0, 0),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: LOS config failed (%d).\n", err);
        return err;
    }

    /* Step 2: internal ~2.703 GHz VCO (22 x 122.88 MHz = 2703.36 MHz, R2=1 N2=11 PFD2=245.76 MHz) */
    if (err = adi_hmc7044_vco_sel_set(dev, HMC7044_VCO_INTERNAL_3GHZ, 0),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: VCO select failed (%d).\n", err);
        return err;
    }

    /* Step 3: force CH_13 to continuous clock mode before adi_hmc7044_clk_config reads
     * startup_mode.  The init table leaves CH_13 startup_mode=1 (SYSREF), which causes
     * adi_hmc7044_clk_config to apply sysref_timer as the divider and set force_mute=2.
     * Clearing startup_mode to 0 here makes the API take the clock branch instead:
     * divider = VCO/7.68MHz = 352, force_mute = 0. */
    if (err = adi_hmc7044_output_sync_config_set(dev, 13, 0, 0, 1),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: CH_13 startup_mode set failed (%d).\n", err);
        return err;
    }

    /* Step 4: program PLL1/PLL2 dividers and output channel dividers */
    if (err = adi_hmc7044_clk_config(dev, HMC7044_CLK_IN_0, pri,
              ref_hz, ref_hz, hmc_out_ch, hmc_out_hz),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: clk_config failed (%d)%s.\n", err,
               (err == API_CMS_ERROR_INVALID_PARAM)
               ? " -- check LATTICE_HMC7044_REF_CLK_HZ / LATTICE_AD9986_DEV_CLK_HZ" : "");
        return err;
    }

    /* Steps 5-8: SYSREF enable, high-performance outputs, register commit, reseed */
    if (err = adi_hmc7044_device_sysref_enable_control_set(dev, 1, 1),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: sysref enable failed (%d).\n", err);
        return err;
    }
    if (err = adi_hmc7044_high_performance_set(dev), err != API_CMS_ERROR_OK) {
        printf("HMC7044: high-performance mode failed (%d).\n", err);
        return err;
    }
    if (err = adi_hmc7044_reg_update(dev), err != API_CMS_ERROR_OK) {
        printf("HMC7044: register commit (reg_update) failed (%d).\n", err);
        return err;
    }
    if (err = adi_hmc7044_reseed_request_set(dev), err != API_CMS_ERROR_OK) {
        printf("HMC7044: reseed request failed (%d).\n", err);
        return err;
    }

    /* Step 8: allow PLL to settle before the caller checks lock status */
    lattice_wait_us(NULL, 100000);  /* 100 ms */

    printf("HMC7044 clocks configured: ref=%.2f MHz  CH_2=%.2f MHz  CH_3=7.68 MHz (SYSREF)  CH_13=7.68 MHz (continuous)  CH_6=122.88 MHz  CH_12=122.88 MHz.\n",
           (double)ref_hz / 1.0e6,
           (double)LATTICE_AD9986_DEV_CLK_HZ / 1.0e6);
    return API_CMS_ERROR_OK;
}

/* Reset and initialize the HMC7044 to the ADI-recommended register defaults.
 *
 * A soft reset (hw_reset = 0) is used because the HMC7044 device handle on this
 * platform has no reset_pin_ctrl wired; the soft reset is driven through the
 * SPI SW_RESET register. adi_hmc7044_device_reset() already applies the
 * recommended init table after the reset; adi_hmc7044_device_init() is then
 * called explicitly to make the initialization step self-evident and to allow
 * re-initialization without a further reset.
 *
 * This must run BEFORE app_hmc7044_enable_sdo(), as the reset returns register
 * 0x0050 (GPO1 routing) to its default and would otherwise clear the SDO
 * configuration required for 4-wire read-back. */
static int32_t app_hmc7044_init(adi_hmc7044_device_t *dev)
{
    int32_t err;

    if (err = adi_hmc7044_device_reset(dev, 0 /* soft reset */), err != API_CMS_ERROR_OK) {
        printf("HMC7044: reset failed (%d).\n", err);
        return err;
    }
    if (err = adi_hmc7044_device_init(dev), err != API_CMS_ERROR_OK) {
        printf("HMC7044: init (recommended register table) failed (%d).\n", err);
        return err;
    }
    printf("HMC7044 reset and initialized to recommended defaults.\n");
    return API_CMS_ERROR_OK;
}

/* ── PLL monitor (background thread) ─────────────────────────────────────
 *
 * Polls HMC7044 and AD9986 clock PLL lock status every PLL_MONITOR_INTERVAL_S
 * seconds.  Only prints when status changes from the last observed value;
 * a WARNING is emitted on any unlock and a recovery notice on re-lock.
 *
 * Override the interval at build time: -DPLL_MONITOR_INTERVAL_S=<n>
 * Acquire g_spi_mtx around any SPI API call outside of startup. */

#ifndef PLL_MONITOR_INTERVAL_S
#define PLL_MONITOR_INTERVAL_S 5
#endif

typedef struct {
    adi_hmc7044_device_t *hmc;
    adi_ad9986_device_t  *ad9;
} pll_mon_ctx_t;

static volatile int    g_mon_stop = 0;
static uint8_t g_hmc_pll_state    = 0xFF; /* 0xFF = not yet seeded */
static uint8_t g_ad9_pll_state    = 0xFF;

static void *pll_monitor_thread(void *arg)
{
    pll_mon_ctx_t *ctx = (pll_mon_ctx_t *)arg;

    while (!g_mon_stop) {
        int     tick;
        uint8_t hmc_st;
        uint8_t ad9_st;

        /* Sleep in 100 ms slices so the thread reacts promptly to g_mon_stop. */
        for (tick = 0; tick < PLL_MONITOR_INTERVAL_S * 10 && !g_mon_stop; tick++)
            usleep(100000);

        if (g_mon_stop)
            break;

        hmc_st = 0;
        ad9_st = 0;

        pthread_mutex_lock(&g_spi_mtx);

        if (adi_hmc7044_device_pll_lock_status_get(ctx->hmc, &hmc_st) == API_CMS_ERROR_OK
                && hmc_st != g_hmc_pll_state) {
            char dec[96];
            uint8_t r7c = 0, r7d = 0;
            if (!(hmc_st & HMC7044_PLL1_AND_PLL2_LOCK_ST))
                printf("\n[PLL MONITOR] WARNING: HMC7044 PLL not locked\n");
            else
                printf("\n[PLL MONITOR] HMC7044 PLL lock restored\n");
            adi_hmc7044_device_spi_register_get(ctx->hmc, 0x007C, &r7c);
            adi_hmc7044_device_spi_register_get(ctx->hmc, 0x007D, &r7d);
            snprintf(dec, sizeof(dec), "[5]=PLL1_lock=%u  API=%s",
                     (r7c >> 5) & 1, (hmc_st & HMC7044_PLL1_LOCK_ST) ? "LOCKED" : "NOT LOCKED");
            app_reg_print_u8(0x007C, "HMC7044_PLL1_STATUS", r7c, dec);
            snprintf(dec, sizeof(dec), "[0]=PLL2_lock=%u  API=%s",
                     r7d & 1, (hmc_st & HMC7044_PLL2_LOCK_ST) ? "LOCKED" : "NOT LOCKED");
            app_reg_print_u8(0x007D, "HMC7044_PLL2_STATUS", r7d, dec);
            g_hmc_pll_state = hmc_st;
        }

        if (adi_ad9986_device_clk_pll_lock_status_get(ctx->ad9, &ad9_st) == API_CMS_ERROR_OK
                && ad9_st != g_ad9_pll_state) {
            char dec[96];
            if (ad9_st != 0x03) {
                printf("\n[PLL MONITOR] WARNING: AD9986 clock PLL not locked\n");
                snprintf(dec, sizeof(dec), "[0]=BF_PLL_LOCK_SLOW=%s  [1]=BF_PLL_LOCK_FAST=%s",
                         (ad9_st & 0x01) ? "LOCKED" : "NOT LOCKED",
                         (ad9_st & 0x02) ? "LOCKED" : "NOT LOCKED");
            } else {
                printf("\n[PLL MONITOR] AD9986 clock PLL lock restored\n");
                snprintf(dec, sizeof(dec), "[0]=BF_PLL_LOCK_SLOW=LOCKED  [1]=BF_PLL_LOCK_FAST=LOCKED");
            }
            app_reg_print_u8(0x2008, "REG_CLK_PLL_STATUS", ad9_st, dec);
            g_ad9_pll_state = ad9_st;
        }

        pthread_mutex_unlock(&g_spi_mtx);
    }
    return NULL;
}

static const char *prbs_pattern_name(adi_cms_jesd_prbs_pattern_e prbs)
{
    switch (prbs) {
    case PRBS7:  return "PRBS7";
    case PRBS9:  return "PRBS9";
    case PRBS15: return "PRBS15";
    case PRBS23: return "PRBS23";
    case PRBS31: return "PRBS31";
    default:     return "PRBS?";
    }
}

/* Run an AD9986 JRX PHY PRBS loopback test with a caller-supplied pattern.
 *
 * Sequence:
 *   1. Stop the FPGA JTX transmitter.
 *   2. Configure all 16 FPGA JTX lanes to emit the requested PRBS pattern.
 *   3. Start the FPGA JTX transmitter (includes a 1-second PLL settle wait).
 *   4. Enable the AD9986 JRX PHY PRBS checker for 100 ms.
 *   5. Read and report per-lane error counts (lanes 0-7).
 *   6. Disable the AD9986 PRBS checker and restore the FPGA to idle.
 *
 * prbs: one of PRBS7, PRBS9, PRBS15, PRBS23, PRBS31 (adi_cms_jesd_prbs_pattern_e).
 *
 * Lane results are informational: a FAIL only returns from this function if
 * an API register access fails, not if individual lanes report PRBS errors.
 * (JESD204 link bring-up is required to guarantee all lanes pass.) */
static int32_t app_ad9986_prbs_test(adi_ad9986_device_t *dev,
                                    adi_cms_jesd_prbs_pattern_e prbs)
{
    int32_t                err;
    int                    lane;
    adi_ad9986_prbs_test_t result;
    int                    pass_cnt = 0;
    const int              NUM_LANES = 8;

    printf("AD9986: JRX PHY %s test (%d lanes, 100 ms).\n",
           prbs_pattern_name(prbs), NUM_LANES);

    adi_ads9_reg_set(0x537, 4);  /* stop any active GT TX pattern play */
    if (err = adi_ads9_stop_transmit(), err != API_CMS_ERROR_OK) {
        printf("AD9986 PRBS: FPGA stop transmit failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_config_jtx_prbs(prbs), err != API_CMS_ERROR_OK) {
        printf("AD9986 PRBS: FPGA JTX %s config failed (%d).\n",
               prbs_pattern_name(prbs), err);
        goto restore;
    }
    if (err = adi_ads9_start_transmit(), err != API_CMS_ERROR_OK) {
        printf("AD9986 PRBS: FPGA start transmit failed (%d).\n", err);
        goto restore;
    }
    if (err = adi_ad9986_jesd_rx_phy_prbs_test(dev, prbs, 100), err != API_CMS_ERROR_OK) {
        printf("AD9986 PRBS: JRX PHY test enable failed (%d).\n", err);
        goto restore;
    }

    for (lane = 0; lane < NUM_LANES; lane++) {
        char lane_name[32];
        char dec[96];
        uint8_t r_test3 = 0;

        if (err = adi_ad9986_jesd_rx_phy_prbs_test_result_get(dev, lane, &result),
            err != API_CMS_ERROR_OK) {
            printf("AD9986 PRBS: lane %d result read failed (%d).\n", lane, err);
            goto restore;
        }
        adi_ad9986_hal_reg_get(dev, 0x0953 + lane, &r_test3);
        snprintf(lane_name, sizeof(lane_name), "REG_JRX_TEST_3_LANE%d", lane);
        snprintf(dec, sizeof(dec),
                 "[6]=BF_JRX_PRBS_LANE_INVALID_DATA_FLAG=%u  [7]=BF_JRX_PRBS_LANE_ERROR_FLAG=%u"
                 "  API_pass=%s  API_err_cnt=%u",
                 (r_test3 >> 6) & 1, (r_test3 >> 7) & 1,
                 result.phy_prbs_pass ? "YES" : "NO", result.phy_prbs_err_cnt);
        app_reg_print_u8(0x0953 + lane, lane_name, r_test3, dec);
        if (result.phy_prbs_pass)
            pass_cnt++;
    }

    printf("AD9986 PRBS: %d/%d lanes passed.\n", pass_cnt, NUM_LANES);
    err = API_CMS_ERROR_OK;

restore:
    adi_ad9986_jesd_rx_phy_prbs_test_disable_set(dev);
    adi_ads9_config_jtx_prbs(PRBS_NONE);
    adi_ads9_stop_transmit();
    return err;
}

/* Enable AD9986 JTX PHY PRBS7 on link 0.
 * Puts all active JTX serializer lanes into PRBS7 test-pattern mode so the
 * FPGA RX can check the pattern with its own PRBS checker.
 * Call app_ad9986_jtx_prbs_stop() to return to normal JESD data. */
static int32_t app_ad9986_jtx_prbs_start(adi_ad9986_device_t *dev)
{
    int32_t err;
    /* Inject PRBS7 at the JTX framer TRANSPORT-LAYER (sample) input — not the PHY.
     * data_source = AD9986_JESD_TX_TEST_DATA_SAMPLE routes PRBS7 through the framer,
     * lane crossbar, scrambler (off here) and 64B/66B encoder, so the FPGA deframer
     * recovers PRBS7 on the sample stream.  This exercises the full framer datapath.
     * Prerequisite: the JESD204C link must be UP (run option 4, confirm with option 3)
     * before enabling, because sample-source PRBS travels the whole link. */
    if (err = adi_ad9986_jesd_tx_gen_test(dev, AD9986_LINK_0,
              AD9986_JESD_TX_TEST_DATA_SAMPLE, AD9986_JESD_TX_TEST_MODE_PN7),
        err != API_CMS_ERROR_OK) {
        printf("AD9986 JTX PRBS: enable sample-PRBS7 failed (%d).\n", err);
        return err;
    }
    printf("AD9986 JTX framer PRBS7 at transport-layer input active — FPGA deframer should see PRBS7.\n");
    return API_CMS_ERROR_OK;
}

/* Disable AD9986 JTX PHY PRBS and restore normal JESD data output. */
static int32_t app_ad9986_jtx_prbs_stop(adi_ad9986_device_t *dev)
{
    int32_t err;
    if (err = adi_ad9986_jesd_tx_gen_test(dev, AD9986_LINK_0,
              AD9986_JESD_TX_TEST_DATA_SAMPLE, AD9986_JESD_TX_TEST_MODE_DISABLED),
        err != API_CMS_ERROR_OK) {
        printf("AD9986 JTX PRBS: disable failed (%d).\n", err);
        return err;
    }
    printf("AD9986 JTX framer PRBS disabled — TX returning to normal JESD sample data.\n");
    return API_CMS_ERROR_OK;
}

/* Shared JESD204C bring-up sequence for the Raspberry Pi control plane.
 *
 * Every datapath and link parameter is read directly from the use-case tables
 * in uc_settings.c at index AD9986_UC_INDEX; there are no parameter literals in
 * this file.  Index 1 selects the JESD204C use case (dev_ref=122.88 MHz,
 * DAC=7864.32 MHz, ADC=3932.16 MHz; JRX and JTX = 1L 4F 2M 1S, K=64, subclass 1,
 * jesd_jesdv=2).  Both the initial bring-up in main() and menu option 4 call
 * this single routine, so the sequence exists in exactly one place. */
static int32_t app_ad9986_jesd204c_bringup(adi_ad9986_device_t *dev)
{
    int32_t err;

    printf("AD9986: configuring JESD204C (uc_settings index %d)...\n", AD9986_UC_INDEX);

    /* Step 1: program FPGA JESD204C link parameters (uc_settings.c index 1). */
    {
        adi_cms_jesd_param_t jrx_param_fpga[2] = { jrx_param[AD9986_UC_INDEX], { 0 } };
        if (err = adi_ads9_config_jesd(jrx_param_fpga, jtx_param[AD9986_UC_INDEX]), err != API_CMS_ERROR_OK) {
            printf("FPGA: JESD204C config failed (%d).\n", err);
            return err;
        }
    }
    printf("FPGA: JESD IP configured for JESD204C.\n");

    /* Step 2: FPGA SYSREF source and continuous re-alignment registers.
     * sysref_config(0) = FMC-routed SYSREF (HMC7044 CH_13 → FPGA SYSREF pin).
     * 0x602 / 0x202 = jesd204c_tx/rx_sysref_always: re-align every LEMC boundary
     * in SYSREF_CONT mode (required for subclass-1 204C). */
    if (err = adi_ads9_sysref_config(0), err != API_CMS_ERROR_OK) {
        printf("FPGA: sysref_config failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_reg_set(0x602, 0x0001), err != API_CMS_ERROR_OK) {
        printf("FPGA: 0x602 (tx_sysref_always) write failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_reg_set(0x202, 0x0001), err != API_CMS_ERROR_OK) {
        printf("FPGA: 0x202 (rx_sysref_always) write failed (%d).\n", err);
        return err;
    }
    /* Set FPGA BRAM capture size and TX pattern player address/length so the
     * GT pattern player (0x537=1) sends repeating data rather than 0 bytes.
     * Without these the pattern length register defaults to 0 and no data is
     * transmitted, preventing CDR lock on the AD9986 JRX deserializer.
     * Matches the ADS9 reference app (app_ads9/ad9986_app.c lines 403-408). */
    if (err = adi_ads9_capture_size_set(0x20), err != API_CMS_ERROR_OK) {
        printf("FPGA: capture_size_set failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_pattern_addr_set(0x80000000), err != API_CMS_ERROR_OK) {
        printf("FPGA: pattern_addr_set failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_pattern_len_set(0x8000), err != API_CMS_ERROR_OK) {
        printf("FPGA: pattern_len_set failed (%d).\n", err);
        return err;
    }
    printf("FPGA: SYSREF source, alignment registers, and pattern player configured.\n");

    /* Step 3: FPGA TX SerDes lane driver for 204C at 16.22 Gbps. */
    if (err = adi_ads9_jesd_tx_lane_driver_config(0xFF, 0x0, 0x0, 0x14),
        err != API_CMS_ERROR_OK) {
        printf("FPGA: TX lane driver config failed (%d).\n", err);
        return err;
    }
    printf("FPGA: TX lane driver configured for 16.22 Gbps 204C.\n");

    /* Step 4: First bidir_start — put FPGA TX in skip-data mode and start
     * bidirectional synchronization.  The bidir_start triggers the FPGA JESD TX
     * state machine to generate proper JESD204C sync headers.  Without this the
     * FPGA transmits unframed data and the AD9986 JRX link layer cannot lock. */
    if (err = adi_ads9_reg_set(0x540, 1), err != API_CMS_ERROR_OK) {
        printf("FPGA: transmit_skip_data write failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_reg_set(0x106, 0x000), err != API_CMS_ERROR_OK) {
        printf("FPGA: skip_rx_link_init=0 write failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_reg_set(0x947, 2), err != API_CMS_ERROR_OK) {
        printf("FPGA: bidir_start (1st) write failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_reg_set(0x106, 0x400), err != API_CMS_ERROR_OK) {
        printf("FPGA: skip_rx_link_init=1 write failed (%d).\n", err);
        return err;
    }
    printf("FPGA: first bidir_start complete — JESD TX generating 204C sync headers.\n");

    /* Step 5: AD9986 JRX bring-up with FPGA TX sending 204C sync headers. */
    if (err = adi_ad9986_device_startup_tx(dev,
              tx_interp[AD9986_UC_INDEX][0] /*main_interp*/, tx_interp[AD9986_UC_INDEX][1] /*chan_interp*/,
              tx_dac_chan_xbar[AD9986_UC_INDEX], tx_main_shift[AD9986_UC_INDEX],
              tx_chan_shift[AD9986_UC_INDEX], &jrx_param[AD9986_UC_INDEX]), err != API_CMS_ERROR_OK) {
        printf("AD9986: startup_tx (JRX/DAC path) failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JRX (DAC path) JESD204C configured. JRX lane rate = %lu bps (%.3f Gbps).\n",
           (unsigned long)dev->dev_info.jesd_rx_lane_rate,
           (double)dev->dev_info.jesd_rx_lane_rate / 1.0e9);

    /* Step 6: Explicitly enable JRX link 0 so the AD9986 JRX state machine
     * can progress from INIT state toward LINK_DATA. */
    if (err = adi_ad9986_jesd_rx_link_enable_set(dev, AD9986_LINK_0, 1),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link 0 enable failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JRX link 0 enabled.\n");

    /* Step 7: AD9986 JTX bring-up (ADC → FPGA RX path). */
    if (err = adi_ad9986_device_startup_rx(dev,
              rx_cddc_select[AD9986_UC_INDEX] /*cddcs*/,
              rx_fddc_select[AD9986_UC_INDEX] /*fddcs*/,
              rx_cddc_shift[AD9986_UC_INDEX], rx_fddc_shift[AD9986_UC_INDEX],
              rx_cddc_dcm[AD9986_UC_INDEX], rx_fddc_dcm[AD9986_UC_INDEX],
              rx_cddc_c2r[AD9986_UC_INDEX], rx_fddc_c2r,
              jtx_param[AD9986_UC_INDEX], jtx_conv_sel[AD9986_UC_INDEX]), err != API_CMS_ERROR_OK) {
        printf("AD9986: startup_rx (JTX/ADC path) failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JTX (ADC path) JESD204C configured.\n");

    /* Step 8: Subclass-1 SYSREF synchronization.
     * Configure AD9986 SYSREF input (AC-coupled CML differential), arm CDDC NCO
     * sync on SYSREF, then perform oneshot sync to capture the LEMC boundary. */
    if (err = adi_ad9986_sync_sysref_input_config_set(dev,
              COUPLING_AC, SIGNAL_CML, 0, 0), err != API_CMS_ERROR_OK) {
        printf("AD9986: SYSREF input config failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_adc_ddc_coarse_sync_enable_set(dev,
              AD9986_ADC_CDDC_ALL, 1), err != API_CMS_ERROR_OK) {
        printf("AD9986: CDDC coarse_sync_enable failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_adc_ddc_coarse_sync_next_set(dev,
              AD9986_ADC_CDDC_ALL, 1), err != API_CMS_ERROR_OK) {
        printf("AD9986: CDDC coarse_sync_next failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_adc_ddc_coarse_trig_nco_reset_enable_set(dev,
              AD9986_ADC_CDDC_ALL, 0), err != API_CMS_ERROR_OK) {
        printf("AD9986: CDDC trig_nco_reset_enable failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_jesd_oneshot_sync(dev, JESD_SUBCLASS_1),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: oneshot_sync failed (%d)%s.\n", err,
               (err == API_CMS_ERROR_JESD_SYNC_NOT_DONE) ? " (SYSREF not captured)" : "");
        return err;
    }
    printf("AD9986: SYSREF oneshot sync complete (LEMC aligned).\n");

    /* Step 9: Explicitly enable JTX link 0. */
    if (err = adi_ad9986_jesd_tx_link_enable_set(dev, AD9986_LINK_0, 1),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JTX link 0 enable failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JTX link 0 enabled.\n");

    /* Step 10: Second bidir_start + 100 ms wait.  Re-asserts bidirectional sync
     * after both JRX and JTX links are enabled and SYSREF is aligned. */
    if (err = adi_ads9_reg_set(0x106, 0x000), err != API_CMS_ERROR_OK) {
        printf("FPGA: skip_rx_link_init=0 (2nd bidir) failed (%d).\n", err);
        return err;
    }
    if (err = adi_ads9_reg_set(0x947, 2), err != API_CMS_ERROR_OK) {
        printf("FPGA: bidir_start (2nd) write failed (%d).\n", err);
        return err;
    }
    lattice_wait_us(NULL, 100000);
    if (err = adi_ads9_reg_set(0x106, 0x400), err != API_CMS_ERROR_OK) {
        printf("FPGA: skip_rx_link_init=1 (2nd bidir) failed (%d).\n", err);
        return err;
    }
    printf("FPGA: second bidir_start complete.\n");

    /* Step 11: Stop GT pattern play and reset JRX link; keep bidir+skip_data.
     *
     * On the RPi board, BRAM at 0x80000000 is NOT pre-loaded with valid
     * 66b/64b JESD data (unlike the ADS9 reference board).  Starting the GT
     * pattern player (0x537=1) sends raw uninitialized BRAM bytes directly
     * to the SerDes, BYPASSING the JESD 66b/64b encoder — no valid sync
     * headers reach the AD9986 JRX.
     *
     * Instead, keep the TX in bidir+skip_data mode (0x947=2 from step 10,
     * 0x540=0x01 from step 4).  In this mode the JESD IP encoder owns the
     * TX path and inserts proper 66b/64b sync headers on every skip block,
     * which is exactly what the AD9986 JRX needs for block sync. */
    if (err = adi_ads9_reg_set(0x537, 4), err != API_CMS_ERROR_OK) {
        printf("FPGA: gt_tx_ptn_play_stop failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_jesd_rx_link_enable_set(dev, AD9986_LINK_ALL, 0),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link disable (pre-cal) failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_jesd_rx_link_enable_set(dev, AD9986_LINK_0, 1),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link re-enable (pre-cal) failed (%d).\n", err);
        return err;
    }
    printf("FPGA: GT play stopped, bidir+skip_data active — JESD encoder generates sync headers.\n");

    /* Step 12: mandatory JESD204C PHY foreground calibration. */
    if (err = adi_ad9986_jesd_rx_calibrate_204c(dev,
              1 /*force_cal_reset*/, 0x00 /*boost_mask: 0=no boost, matches ref app*/,
              0 /*run_bg_cal*/), err != API_CMS_ERROR_OK) {
        printf("AD9986: JESD204C JRX PHY calibration failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JRX JESD204C PHY calibration complete.\n");

    /* Step 13: Reset JRX link after calibration: disable all links, re-enable
     * link 0.  Done twice to flush calibration state (matches ADS9 reference). */
    if (err = adi_ad9986_jesd_rx_link_enable_set(dev, AD9986_LINK_ALL, 0),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link disable (post-cal 1) failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_jesd_rx_link_enable_set(dev, AD9986_LINK_0, 1),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link re-enable (post-cal 1) failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_jesd_rx_link_enable_set(dev, AD9986_LINK_ALL, 0),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link disable (post-cal 2) failed (%d).\n", err);
        return err;
    }
    if (err = adi_ad9986_jesd_rx_link_enable_set(dev, AD9986_LINK_0, 1),
        err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link re-enable (post-cal 2) failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JRX link reset after calibration complete.\n");

    /* Step 14: Poll JRX 204C state every 2 s for 20 s.  Print AD9986 0x055E only. */
    {
        int      tick;
        uint16_t jrx_poll = 0;
        uint8_t  r055E = 0;
        char dec[96];
        printf("Polling JRX 204C state (every 2 s for 20 s):\n");
        for (tick = 0; tick < 10; tick++) {
            lattice_wait_us(NULL, 2000000);
            adi_ad9986_jesd_rx_link_select_set(dev, AD9986_LINK_0);
            adi_ad9986_hal_reg_get(dev, 0x055E, &r055E);
            adi_ad9986_jesd_rx_link_status_get(dev, AD9986_LINK_0, &jrx_poll);
            (void)jrx_poll;
            printf("  t=%2ds:\n", (tick + 1) * 2);
            snprintf(dec, sizeof(dec),
                     "[7]=BF_JRX_DL_204C_ENABLE=%u  [6:4]=BF_JRX_DL_204C_STATE=%u  %s",
                     (r055E >> 7) & 1, (r055E >> 4) & 7,
                     ((r055E >> 4) & 7) == 6 ? "(link up)" : "(link down)");
            app_reg_print_u8(0x055E, "REG_JRX_DL_204C_0", r055E, dec);
            if (((r055E >> 4) & 7) == 6) {
                printf("  JRX reached LINK_DATA (state=6)!\n");
                break;
            }
        }
    }

    printf("AD9986: JESD204C (index 1) configuration complete.\n");
    return API_CMS_ERROR_OK;
}

int main(int argc, char *argv[])
{
    int32_t       err;
    int32_t       readback;
    pthread_t     mon_thread;
    int           mon_started = 0;
    pll_mon_ctx_t mon_ctx;

    /* AD9986 on SPI0 CS1 (proxied by the Lattice FPGA), 4-wire, MSB first. */
    adi_ad9986_device_t ad9986_dev = {
        .hal_info = {
            .sdo            = SPI_SDO,
            .msb            = SPI_MSB_FIRST,
            .addr_inc       = SPI_ADDR_INC_AUTO,
            .log_write      = lattice_log_write,
            .delay_us       = lattice_wait_us,
            .spi_xfer       = lattice_spi_xfer_ad9986,
            .reset_pin_ctrl = lattice_hw_rst_pin_ctrl_ad9986,
        },
        .clk_info = {
            .sysref_mode = SYSREF_CONT,
        },
        .serdes_info = {
            /* JTX: AD9986 ADC serializer → FPGA RX.
             * index = physical AD9986 JTX lane, value = logical lane it carries.
             * Only physical lanes whose value < jesd_l are powered up.
             * For L=1: only the entry equal to 0 selects the active physical lane.
             * Update index N to 0 (and set all others to a value >= L) once the
             * board schematic confirms which AD9986 JTX pin connects to the FPGA RX. */
            .ser_settings = {
                .lane_mapping = {
                    /* link0 identity: logical 0 -> physical SERDOUT0.
                     * Schematic 02-051884-01-f p6/p12: AD9986 SERDOUT0 = net FPGA_SERDIN_0
                     * = FMC DP2_M2C = FPGA balls L34/M34 = rx_serial_data[0].  Identity is
                     * REQUIRED so the framer emits on the lane the FPGA RX listens to.
                     * (Was {1,0,...}, which put logical 0 on SERDOUT1 = DP0_M2C = dead lane.) */
                    { 7, 0, 7, 7, 7, 7, 7, 7 },
                    //x2{ 7, 0, 7, 7, 7, 7, 7, 1 },
                    { 4, 5, 6, 7, 0, 1, 2, 3 }, /* link1 (unused for single-link) */
                },
            },
            /* JRX: FPGA TX → AD9986 DAC deserializer.
             * index = physical AD9986 JRX lane, value = logical lane it carries.
             * Only physical lanes whose value < jesd_l are powered up.
             * For L=1: only the entry equal to 0 selects the active physical lane.
             * Update index M to 0 (and set all others to a value >= L) once the
             * board schematic confirms which AD9986 JRX pin the FPGA TX drives. */
            .des_settings = {
                /* CTLE filter 1-4 (valid range); 2 = mid insertion-loss setting.
                 * Required for QUART_RATE (JESD204C > 16 Gbps) deserializer path. */
                .boost_mask  = 0xff,
                .ctle_filter = { 2, 2, 2, 2, 2, 2, 2, 2 },
                .lane_mapping = {
                    { 0, 7, 7, 7, 7, 7, 7, 7 }, /* link0: identity — phys i → logical i */
                    //x2{ 0, 7, 7, 7, 1, 7, 7, 7 },
                    { 4, 5, 6, 7, 0, 1, 2, 3 }, /* link1 (unused for single-link) */
                },
            },
        },
    };

    /* HMC7044 on SPI1. */
    adi_hmc7044_device_t hmc7044_dev = {
        .hal_info = {
            .spi_xfer = lattice_spi_xfer_hmc7044,
            .delay_us = lattice_wait_us,
        }
    };

    (void)argc;
    (void)argv;

    printf("AD9986 control-plane app (Raspberry Pi 5 + Lattice FPGA proxy)\n");
    printf("  SPI0 CS0 -> JESD IP : %s\n", LATTICE_SPI_JESD_DEV);
    printf("  SPI0 CS1 -> AD9986  : %s\n", LATTICE_SPI_AD9986_DEV);
    printf("  SPI1 CS0 -> HMC7044 : %s\n", LATTICE_SPI_HMC7044_DEV);

    if (err = lattice_hw_open("ad9986_rpi_app.log"), err != API_CMS_ERROR_OK) {
        printf("platform open failed (%d).\n", err);
        return err;
    }
    if (err = lattice_user_data_create_ad9986(&ad9986_dev, 1 /*SPI0 CS1*/), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }
    if (err = lattice_user_data_create_hmc7044(&hmc7044_dev, 0 /*SPI1 CS0*/), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI0 CS0: confirm the JESD IP register bridge is reachable. */
    if (err = app_jesd_ip_init(), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI1: reset and initialize the HMC7044 to recommended defaults. This is
     * write-only and must precede the SDO routing (the reset clears it). */
    if (err = app_hmc7044_init(&hmc7044_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI1: enable 4-wire read-back by routing the HMC7044 SDO onto GPIO1.
     * This is write-only (3-wire write path) and must precede any read. */
    if (err = app_hmc7044_enable_sdo(&hmc7044_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI1: configure HMC7044 PLL and output clocks.
     * Programs PLL1/PLL2 dividers and output channel 2 (AD9986 device clSelect an option:
  1 - Run JESD IP registock).
     * Reference clock and device clock frequencies are set in lattice.h via
     * LATTICE_HMC7044_REF_CLK_HZ and LATTICE_AD9986_DEV_CLK_HZ; includes
     * a 100 ms settle wait before returning. */
    if (err = app_hmc7044_clk_config(&hmc7044_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI1: clock generator identity (non-fatal; SDO must be enabled first). */
    app_hmc7044_identify(&hmc7044_dev);

    /* SPI1: verify HMC7044 PLL1 and PLL2 are locked after clock configuration.
     * Required before trusting any clock output to downstream devices. */
    if (err = app_hmc7044_pll_lock_check(&hmc7044_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI0 CS1: reset, initialize and identify the AD9986 through the proxy. */
    if (err = app_ad9986_identify(&ad9986_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI0 CS1: configure the AD9986 on-chip clock PLL.
     * Programs the device PLL for the DAC and ADC sample rates defined in
     * lattice.h (LATTICE_AD9986_DAC_CLK_HZ / LATTICE_AD9986_ADC_CLK_HZ). */
    if (err = app_ad9986_clk_config(&ad9986_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI0 CS1: verify the AD9986 clock PLL is locked.
     * status 0x3 = PLL_LOCK_FAST (bit0) + PLL_LOCK_SLOW (bit1). */
    if (err = app_ad9986_clk_pll_lock_check(&ad9986_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* SPI0 CS1: enable the ADCDRV differential output (ADC_CLK SMA on board).
     * PD_ADC_DRIVER defaults to 1 (powered down); this clears it so the
     * ADCDRV_P/N pins drive the ADC sample clock through the board balun. */
    if (err = adi_ad9986_adc_clk_out_enable_set(&ad9986_dev, 1), err != API_CMS_ERROR_OK) {
        printf("AD9986: ADCDRV output enable failed (%d).\n", err);
        goto cleanup;
    }
    printf("AD9986: ADCDRV output enabled (ADC_CLK SMA active, %.2f MHz).\n",
           (double)LATTICE_AD9986_ADC_CLK_HZ / 1.0e6);

    /* SPI0 CS1: configure AD9986 JESD204C datapath (uc_settings index 1 params).
     * Requires AD9986 clock PLL locked (above) and HMC7044 CH_13 SYSREF active (above). */
    if (err = app_ad9986_jesd204c_bringup(&ad9986_dev), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* Seed PLL monitor state from current known-good values so the first poll
     * only prints on a change, then start the background thread. */
    {
        uint8_t st = 0;
        if (adi_hmc7044_device_pll_lock_status_get(&hmc7044_dev, &st) == API_CMS_ERROR_OK)
            g_hmc_pll_state = st;
        if (adi_ad9986_device_clk_pll_lock_status_get(&ad9986_dev, &st) == API_CMS_ERROR_OK)
            g_ad9_pll_state = st;
    }           

#if 1 
    mon_ctx.hmc = &hmc7044_dev;
    mon_ctx.ad9 = &ad9986_dev;
    if (pthread_create(&mon_thread, NULL, pll_monitor_thread, &mon_ctx) == 0) {
        printf("PLL monitor started (polling every %d s"
               "  HMC7044=0x%02x  AD9986=0x%02x).\n",
               PLL_MONITOR_INTERVAL_S, g_hmc_pll_state, g_ad9_pll_state);
        mon_started = 1;
    } else {
        printf("PLL monitor: pthread_create failed -- monitoring disabled.\n");
    }

    /* SPI0 CS0: write/read register verify now that all device clocks are up.
     * The Lattice FPGA JESD IP functional registers are in the device-clock
     * domain; they are writable only after HMC7044 and AD9986 clock PLLs
     * have locked and the clock is reaching the FPGA. */
    {
        int user_choice;
        while (1) {
            printf("\nSelect an option:\n");
            printf("  1 - Run JESD IP register verify\n");
            printf("  2 - Run JESD IP register read\n");
            printf("  3 - Check AD9986 JESD link status (includes JRX/JTX lane rates)\n");
            printf("  4 - Configure AD9986 JESD204C (UC1 params, 7864.32 MHz clock)\n");
            printf("  5 - Check HMC7044 CH_13 SYSREF status\n");
            printf("  6 - Unmute HMC7044 CH_13 SYSREF output (clear force_mute)\n");
            printf("  7 - Start AD9986 JTX framer PRBS7 at transport-layer input (ADC path → FPGA RX)\n");
            printf("  8 - Stop  AD9986 JTX framer PRBS  (restore normal JESD sample data)\n");
            printf("  9 - Read AD9986 204C error IRQ status once\n");
            printf("  10 - AD9986 JRX sample PRBS7 checker (FPGA→AD9986, runs until error or Ctrl+C)\n");
            printf("  11 - Exit\n");
            printf("Enter choice: ");
            if (scanf("%d", &user_choice) != 1) {
                /* flush invalid input or exit on EOF */
                int c;
                if (feof(stdin))
                    break;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("Invalid input. Please enter 1-11.\n");
                continue;
            }
            if (user_choice == 11) {
                break;
            } else if (user_choice >= 1 && user_choice <= 9) {
                pthread_mutex_lock(&g_spi_mtx);
                if (user_choice == 1) {
                    err = app_jesd_ip_reg_verify();
                } else if (user_choice == 2) {
                    app_jesd_ip_reg_read_test();
                } else if (user_choice == 3) {
                    app_ad9986_link_status_check(&ad9986_dev, &hmc7044_dev);
                } else if (user_choice == 4) {
                    app_ad9986_jesd204c_bringup(&ad9986_dev);
                } else if (user_choice == 5) {
                    app_hmc7044_ch13_status(&hmc7044_dev);
                } else if (user_choice == 6) {
                    app_hmc7044_ch13_unmute(&hmc7044_dev);
                } else if (user_choice == 7) {
                    app_ad9986_jtx_prbs_start(&ad9986_dev);
                } else if (user_choice == 8) {
                    app_ad9986_jtx_prbs_stop(&ad9986_dev);
                } else {
                    app_ad9986_irq_once(&ad9986_dev);
                }
                pthread_mutex_unlock(&g_spi_mtx);
            } else if (user_choice == 10) {
                app_ad9986_jrx_sample_prbs(&ad9986_dev);
            } else {
                printf("Invalid choice. Please enter 1-11.\n");
            }
        }
    }      
    
    //if (err = app_jesd_ip_reg_read(0x04, &readback), err != API_CMS_ERROR_OK) {
        //return err;
    //}
    //printf("Address 0x1 read back value: 0x%x\n", readback);

    /* SPI0 CS0/CS1: FPGA JTX → AD9986 JRX PHY PRBS loopback test.
     * Runs after both PLLs are locked and the JESD IP is verified.
     * Change the pattern argument to PRBS9, PRBS15, PRBS23, or PRBS31 as needed. */
    
    pthread_mutex_lock(&g_spi_mtx);
    err = app_ad9986_prbs_test(&ad9986_dev, PRBS7);
    pthread_mutex_unlock(&g_spi_mtx);
    if (err != API_CMS_ERROR_OK) {
        goto cleanup;
    }
#endif          

    /* The full JESD link bring-up is available through the reused adi_ads9_*
     * API, e.g. adi_ads9_config_jesd(jrx_param, jtx_param) followed by the
     * AD9986 datapath startup. Wire in the desired use-case parameters here.
     *
     * After JESD link setup, verify the SerDes PLL before enabling links:
     *     if (err = app_ad9986_jesd_pll_lock_check(&ad9986_dev), ...) goto cleanup;
     */

    printf("Control-plane bring-up complete.\n");

cleanup:
    if (mon_started) {
        g_mon_stop = 1;
        pthread_join(mon_thread, NULL);
        printf("PLL monitor stopped.\n");
    }
    lattice_user_data_free(&ad9986_dev.hal_info.user_data);
    lattice_user_data_free(&hmc7044_dev.hal_info.user_data);
    lattice_hw_close();
    return err;
}
