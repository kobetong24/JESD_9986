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

/*============= C O D E ====================*/
static int32_t app_jesd_ip_reg_write(uint32_t addr, uint32_t data)
{
    int32_t err;

    if (err = adi_ads9_reg_set(addr, data), err != API_CMS_ERROR_OK) {
        printf("JESD IP: write addr=0x%04x failed (%d).\n", addr, err);
        return err;
    }
    printf("JESD IP: write addr=0x%04x  data=0x%08x\n", addr, data);
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
    printf("JESD IP: read  addr=0x%04x  data=0x%08x\n", addr, *data);
    return API_CMS_ERROR_OK;
}

static int32_t app_jesd_ip_init(void)
{
    int32_t  err;
    uint32_t fpga_ver  = 0;

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
    printf("JESD IP (FPGA) image v%x -- SPI0 CS0 bridge OK.\n", fpga_ver);
    return API_CMS_ERROR_OK;
}

static int32_t app_jesd_ip_reg_verify(void)
{
    int32_t  err;
    uint32_t readback = 0;

    /* Write/read verify: addr 0x11D is the JRX subclass register.
     * Called after clocks are up so the JESD IP register domain is active. */
    if (err = app_jesd_ip_reg_write(0x011D, 0xDEADBEEF), err != API_CMS_ERROR_OK) {
        return err;
    }
    if (err = app_jesd_ip_reg_read(0x011D, &readback), err != API_CMS_ERROR_OK) {
        return err;
    }
    if ((readback & 0xFFFFFFFF) != 0xDEADBEEF) {
        printf("JESD IP: register verify FAILED (wrote 0x01 to 0x011D, read 0x%08x).\n",
               readback);
        return API_CMS_ERROR_TEST_FAILED;
    }
    /* Restore JRX subclass to 0 (default = subclass 0) */
    app_jesd_ip_reg_write(0x011D, 0x00000000);
    printf("JESD IP: register write/read verify OK.\n");
    return API_CMS_ERROR_OK;
}

void app_jesd_ip_reg_read_test(void){
	uint32_t readback = 0;

	app_jesd_ip_reg_read(0x40003000, &readback);
	printf("JESD IP: register read value: 0x%x\n", readback);
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
    printf("AD9986 chip type 0x%02x, product id 0x%04x, revision 0x%02x\n",
           chip_id.chip_type, chip_id.prod_id, chip_id.dev_revision);
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
    printf("HMC7044 chip type 0x%02x, product id 0x%04x, revision 0x%02x\n",
           chip_id.chip_type, chip_id.prod_id, chip_id.dev_revision);

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
    printf("HMC7044: PLL lock status 0x%02x  PLL1=%s  PLL2=%s\n",
           status,
           (status & HMC7044_PLL1_LOCK_ST) ? "LOCKED" : "NOT LOCKED",
           (status & HMC7044_PLL2_LOCK_ST) ? "LOCKED" : "NOT LOCKED");
    if (!(status & HMC7044_PLL1_AND_PLL2_LOCK_ST)) {
        printf("HMC7044: PLL not fully locked -- call adi_hmc7044_clk_config() first.\n");
        return API_CMS_ERROR_TEST_FAILED;
    }
    printf("HMC7044: PLL locked OK.\n");
    return API_CMS_ERROR_OK;
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
    printf("AD9986: clock PLL lock status 0x%02x  FAST=%s  SLOW=%s\n",
           status,
           (status & 0x1) ? "LOCKED" : "NOT LOCKED",
           (status & 0x2) ? "LOCKED" : "NOT LOCKED");
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
    uint8_t status = 0;

    if (err = adi_ad9986_jesd_pll_lock_status_get(dev, &status), err != API_CMS_ERROR_OK) {
        printf("AD9986: JESD PLL lock status read failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JESD PLL lock status 0x%02x  %s\n",
           status, status ? "LOCKED" : "NOT LOCKED");
    if (!status) {
        printf("AD9986: JESD SerDes PLL not locked -- check lane rate / JESD clock.\n");
        return API_CMS_ERROR_TEST_FAILED;
    }
    printf("AD9986: JESD PLL locked OK.\n");
    return API_CMS_ERROR_OK;
}

/* Query and print JESD204 link status for both JRX (DAC path) and JTX (ADC path).
 * Can be called any time after clock bring-up; results are only meaningful once
 * the JESD link has been configured and enabled. */
static int32_t app_ad9986_link_status_check(adi_ad9986_device_t *dev)
{
    int32_t  err;
    uint8_t  jesd_pll = 0;
    uint16_t jrx_st   = 0;
    uint16_t jtx_st   = 0;
    int      i;

    /* SerDes PLL */
    if (err = adi_ad9986_jesd_pll_lock_status_get(dev, &jesd_pll), err != API_CMS_ERROR_OK) {
        printf("AD9986: JESD SerDes PLL read failed (%d).\n", err);
        return err;
    }
    printf("AD9986 JESD SerDes PLL : %s\n", jesd_pll ? "LOCKED" : "NOT LOCKED");

    /* JRX (DAC / Rx link: FPGA → AD9986) */
    if (err = adi_ad9986_jesd_rx_link_status_get(dev, AD9986_LINK_0, &jrx_st), err != API_CMS_ERROR_OK) {
        printf("AD9986: JRX link status read failed (%d).\n", err);
        return err;
    }
    printf("AD9986 JRX link 0 (DAC path):\n");
    printf("  204C state : %d %s\n", (jrx_st >> 8) & 0xFF,
           ((jrx_st >> 8) & 0xFF) == 6 ? "(link up)" : "(link down)");
    printf("  Lane status: 0x%02x  ", jrx_st & 0xFF);
    for (i = 0; i < 8; i++)
        printf("L%d:%s ", i, (jrx_st >> i) & 1 ? "UP" : "DN");
    printf("\n");

    /* JTX (ADC / Tx link: AD9986 → FPGA) */
    if (err = adi_ad9986_jesd_tx_link_status_get(dev, AD9986_LINK_0, &jtx_st), err != API_CMS_ERROR_OK) {
        printf("AD9986: JTX link status read failed (%d).\n", err);
        return err;
    }
    printf("AD9986 JTX link 0 (ADC path):\n");
    printf("  QBF state       : 0x%X\n",  jtx_st & 0x0F);
    printf("  Frame sync      : %s\n",    (jtx_st >> 4) & 1 ? "OK"     : "NO SYNC");
    printf("  SerDes PLL      : %s\n",    (jtx_st >> 5) & 1 ? "LOCKED" : "NOT LOCKED");
    printf("  Phase estab.    : %s\n",    (jtx_st >> 6) & 1 ? "YES"    : "NO");
    printf("  Invalid mode    : %s\n",    (jtx_st >> 7) & 1 ? "YES (error)" : "NO (ok)");

    /* JESD standard mode check.
     * JRX (DAC path, not paged):
     *   0x04C0 bit5 = jrx_dl_204b_enable
     *   0x055E bit7 = jrx_dl_204c_enable
     * JTX (ADC path, paged – link 0 already selected above):
     *   0x0642 bits[7:5] = jtx_jesdv_cfg  (0=204A, 1=204B, 2=204C) */
    {
        uint8_t jrx_204b_reg = 0, jrx_204c_reg = 0, jtx_l0_8 = 0;

        adi_ad9986_hal_reg_get(dev, 0x04C0, &jrx_204b_reg);
        adi_ad9986_hal_reg_get(dev, 0x055E, &jrx_204c_reg);

        adi_ad9986_jesd_tx_link_select_set(dev, AD9986_LINK_0);
        adi_ad9986_hal_reg_get(dev, 0x0642, &jtx_l0_8);

        uint8_t jrx_204b_en = (jrx_204b_reg >> 5) & 1;
        uint8_t jrx_204c_en = (jrx_204c_reg >> 7) & 1;
        uint8_t jtx_jesdv   = (jtx_l0_8   >> 5) & 7;

        printf("AD9986 JESD standard:\n");
        printf("  JRX 204B enable (0x04C0[5]) = %d\n", jrx_204b_en);
        printf("  JRX 204C enable (0x055E[7]) = %d\n", jrx_204c_en);
        printf("  JTX JESDV       (0x0642[7:5]) = %d  (%s)\n",
               jtx_jesdv,
               jtx_jesdv == 0 ? "204A" : jtx_jesdv == 1 ? "204B" : jtx_jesdv == 2 ? "204C" : "unknown");
        if (jrx_204b_en && !jrx_204c_en)
            printf("  => JRX configured for JESD204B\n");
        else if (!jrx_204b_en && jrx_204c_en)
            printf("  => JRX configured for JESD204C\n");
        else
            printf("  => JRX: not yet configured (reset defaults)\n");
    }

    /* SYSREF status registers (HMC7044 CH_3 → AD9986 SYSREF input).
     *
     * Deframer (JRX, DAC path) — not paged:
     *   0x04A0 JRX_TPL_0  bit[2] jrx_tpl_sysref_rcvd
     *
     * Framer (JTX, ADC path) — paged, link 0 already selected above:
     *   0x0636 JTX_TPL_6  bit[1] jtx_tpl_sysref_rcvd
     *                      bit[2] jtx_tpl_sysref_phase_err  (subclass 1 only)
     *                      bit[5] jtx_tpl_sysref_mask       (=1 when subclass=0)
     *   0x0667 JTX_DL_204C_0 bit[3] jtx_dl_204c_sysref_rcvd (204C link layer) */
    {
        uint8_t jrx_tpl0 = 0, jtx_tpl6 = 0, jtx_204c_0 = 0;

        adi_ad9986_jesd_rx_link_select_set(dev, AD9986_LINK_0);
        adi_ad9986_hal_reg_get(dev, 0x04A0, &jrx_tpl0);

        adi_ad9986_jesd_tx_link_select_set(dev, AD9986_LINK_0);
        adi_ad9986_hal_reg_get(dev, 0x0636, &jtx_tpl6);
        adi_ad9986_hal_reg_get(dev, 0x0667, &jtx_204c_0);

        printf("AD9986 SYSREF status:\n");
        printf("  Deframer JRX_TPL_0     (0x04A0) = 0x%02x\n", jrx_tpl0);
        printf("    [2] jrx_tpl_sysref_rcvd        = %d\n", (jrx_tpl0 >> 2) & 1);
        printf("  Framer  JTX_TPL_6      (0x0636) = 0x%02x  [paged link0]\n", jtx_tpl6);
        printf("    [1] jtx_tpl_sysref_rcvd        = %d\n", (jtx_tpl6 >> 1) & 1);
        printf("    [2] jtx_tpl_sysref_phase_err   = %d\n", (jtx_tpl6 >> 2) & 1);
        printf("    [5] jtx_tpl_sysref_mask        = %d  (1=masked/subclass0)\n",
               (jtx_tpl6 >> 5) & 1);
        printf("  Framer  JTX_DL_204C_0  (0x0667) = 0x%02x  [paged link0]\n", jtx_204c_0);
        printf("    [3] jtx_dl_204c_sysref_rcvd    = %d\n", (jtx_204c_0 >> 3) & 1);
    }

    return API_CMS_ERROR_OK;
}

/* Read HMC7044 CH_13 output control registers and report SYSREF channel status.
 *
 * CH_13 is the SYSREF output routed to the AD9986 (7.68 MHz from VCO 2703.36 MHz,
 * divider = 352 = 0x160).  There is no single "running" status bit — "up and
 * running" is confirmed by:
 *   - CTRL_0 bit[0] = 1  (channel enable)
 *   - CTRL_0 bits[3:2] = 0  (startup_mode = 0 = continuous, i.e. not held in reset)
 *   - Divider (CTRL_1 + CTRL_2) = 352 = 0x160  (correct for 7.68 MHz output)
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

    printf("HMC7044 CH_13 SYSREF output status:\n");
    printf("  CTRL_0 (0x014A) = 0x%02x\n", ctrl0);
    printf("    [0]   channel enable    = %d  %s\n",
           ctrl0 & 1, (ctrl0 & 1) ? "(enabled)" : "(DISABLED)");
    printf("    [3:2] startup_mode      = %d  %s\n",
           (ctrl0 >> 2) & 3,
           ((ctrl0 >> 2) & 3) == 0 ? "(continuous clock -- not SYSREF mode)"
         : (ctrl0 >> 2) & 3 == 1   ? "(startup pulse generator)"
         : (ctrl0 >> 2) & 3 == 2   ? "(re-arm pulse generator)"
                                   : "(pulse generator mode 3)");
    printf("    [5]   slip_en           = %d\n", (ctrl0 >> 5) & 1);
    printf("    [6]   sync_en           = %d\n", (ctrl0 >> 6) & 1);
    printf("    [7]   high_perform_en   = %d\n", (ctrl0 >> 7) & 1);
    printf("  CTRL_1 (0x014B) = 0x%02x  (divider[7:0])\n", ctrl1);
    printf("  CTRL_2 (0x014C) = 0x%02x  (divider[11:8])\n", ctrl2 & 0x0F);
    printf("  Channel divider = %u  (expected 704 = 0x2C0: sysref_timer for SYSREF channels)\n",
           divider);
    printf("  CTRL_8 (0x0152) = 0x%02x  (driver: force_mute[7:6] drv_mode[4:3] impedance[1:0])\n",
           ctrl8);
    printf("    force_mute      = %d\n", (ctrl8 >> 6) & 3);

    /* Summary */
    int ch_enabled      = (ctrl0 & 1);
    int startup_sysref  = (((ctrl0 >> 2) & 3) != 0); /* non-zero = pulse generator (SYSREF mode) */
    int divider_ok      = (divider == 704);
    int not_muted       = (((ctrl8 >> 6) & 3) == 0);

    printf("HMC7044 CH_13 SYSREF: %s\n",
           (ch_enabled && startup_sysref && divider_ok && not_muted)
           ? "UP AND RUNNING (enabled, SYSREF pulse-gen mode, divider=704, not muted)"
           : "NOT RUNNING (see detail above -- use option 6 to clear force_mute)");

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
    printf("HMC7044 CH_13: CTRL_8 before = 0x%02x  force_mute = %d\n",
           ctrl8, (ctrl8 >> 6) & 3);

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
    printf("HMC7044 CH_13: CTRL_8 after  = 0x%02x  force_mute = %d  %s\n",
           ctrl8,
           (ctrl8 >> 6) & 3,
           ((ctrl8 >> 6) & 3) == 0 ? "(not muted -- output active)"
                                   : "(still muted -- check SPI write path)");
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
     * CH_3/CH_13 → SYSREF 7.68 MHz (VCO/352 = 2703.36/352, divider is even).
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
        7680000ULL                  /* CH_13 → SYSREF 7.68 MHz */
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

    /* Step 3: program PLL1/PLL2 dividers and output channel dividers */
    if (err = adi_hmc7044_clk_config(dev, HMC7044_CLK_IN_0, pri,
              ref_hz, ref_hz, hmc_out_ch, hmc_out_hz),
        err != API_CMS_ERROR_OK) {
        printf("HMC7044: clk_config failed (%d)%s.\n", err,
               (err == API_CMS_ERROR_INVALID_PARAM)
               ? " -- check LATTICE_HMC7044_REF_CLK_HZ / LATTICE_AD9986_DEV_CLK_HZ" : "");
        return err;
    }

    /* Steps 4-6: SYSREF enable, high-performance outputs, register commit, reseed */
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

    printf("HMC7044 clocks configured: ref=%.2f MHz  CH_2=%.2f MHz  CH_3/CH_13=7.68 MHz (SYSREF)  CH_6=122.88 MHz  CH_12=122.88 MHz.\n",
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
        if (err = adi_ad9986_jesd_rx_phy_prbs_test_result_get(dev, lane, &result),
            err != API_CMS_ERROR_OK) {
            printf("AD9986 PRBS: lane %d result read failed (%d).\n", lane, err);
            goto restore;
        }
        printf("AD9986 PRBS: lane %d -- %s  err_cnt=%u\n",
               lane,
               result.phy_prbs_pass ? "PASS" : "FAIL",
               result.phy_prbs_err_cnt);
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

/* Configure the AD9986 for JESD204C using uc_settings.c array index 1 parameters.
 *
 * clk_hz[1]: dev_ref=122.88 MHz, DAC=7864.32 MHz, ADC=3932.16 MHz (unlabeled entry).
 * JRX (FPGA→AD9986 DAC): jrx_param[1] with jesd_jesdv overridden to 2 (204C).
 *   4L 4F 8M 1S K=32 N=16, mode 9, main_interp=4, chan_interp=4, main_shift=1842.5 MHz.
 * JTX (AD9986 ADC→FPGA): jtx_param[1] with jesd_jesdv overridden to 2 (204C).
 *   4L 4F 8M 1S HD=1 K=32 N=16, mode 10, CDDC 0-3 DCM×2, FDDC 0/1/4/5 DCM×4. */
static int32_t app_ad9986_jesd204c_config(adi_ad9986_device_t *dev)
{
    int32_t err;

    /* jrx_param[1]: JESDV field overridden from 1 (204B) to 2 (204C). */
    adi_cms_jesd_param_t jrx_param = {
        .jesd_l        = 4,  .jesd_f      = 4, .jesd_m   = 8,  .jesd_s    = 1,
        .jesd_hd       = 0,  .jesd_k      = 32,
        .jesd_n        = 16, .jesd_np     = 16,
        .jesd_cf       = 0,  .jesd_cs     = 0,
        .jesd_did      = 0,  .jesd_bid    = 0,  .jesd_lid0 = 0,
        .jesd_subclass = 0,  .jesd_scr    = 1,  .jesd_duallink = 0,
        .jesd_jesdv    = 2,  .jesd_mode_id = 9,
    };

    /* jtx_param[1]: JESDV field overridden from 1 (204B) to 2 (204C). */
    adi_cms_jesd_param_t jtx_param[2] = {
        {
            .jesd_l        = 4,  .jesd_f      = 4, .jesd_m   = 8,  .jesd_s    = 1,
            .jesd_hd       = 1,  .jesd_k      = 32,
            .jesd_n        = 16, .jesd_np     = 16,
            .jesd_cf       = 0,  .jesd_cs     = 0,
            .jesd_did      = 0,  .jesd_bid    = 0,  .jesd_lid0 = 0,
            .jesd_subclass = 0,  .jesd_scr    = 1,  .jesd_duallink = 0,
            .jesd_jesdv    = 2,  .jesd_mode_id = 10,
            .jesd_mode_c2r_en = 0, .jesd_mode_s_sel = 0,
        },
        { 0 }
    };

    /* tx_interp[1]: main=4, chan=4; tx_dac_chan_xbar[1]: CH_0..CH_3 */
    uint8_t  dac_chan[4]   = { AD9986_DAC_CH_0, AD9986_DAC_CH_1,
                               AD9986_DAC_CH_2, AD9986_DAC_CH_3 };
    /* tx_main_shift[1]: 1842.5 MHz on each DAC main path */
    int64_t  main_shift[4] = { 1842500000LL, 1842500000LL,
                                1842500000LL, 1842500000LL };
    int64_t  chan_shift[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    /* rx_cddc_shift[1]: CDDC 0/1 = 1842.5 MHz, CDDC 2/3 = 350 MHz */
    int64_t  cddc_shift[4] = { 1842500000LL, 1842500000LL,
                                 350000000LL,  350000000LL };
    int64_t  fddc_shift[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    /* rx_cddc_dcm[1]: all four CDDCs decimate by 2 */
    uint8_t  cddc_dcm[4]   = { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2,
                                AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 };
    /* rx_fddc_dcm[1]: FDDC 0, 1, 4, 5 decimate by 4; slots 2, 3, 6, 7 unused */
    uint8_t  fddc_dcm[8]   = { AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4,
                                0, 0,
                                AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4,
                                0, 0 };
    uint8_t  cc2r_en[4]    = { 0, 0, 0, 0 };
    uint8_t  fc2r_en[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };

    /* jtx_conv_sel[1] link0: M=8 → converters 0-7 = FDDC 0I/0Q/1I/1Q/4I/4Q/5I/5Q */
    adi_ad9986_jtx_conv_sel_t jesd_conv_sel[2] = {
        {
            .virtual_converter0_index = AD9986_FDDC_0_I,
            .virtual_converter1_index = AD9986_FDDC_0_Q,
            .virtual_converter2_index = AD9986_FDDC_1_I,
            .virtual_converter3_index = AD9986_FDDC_1_Q,
            .virtual_converter4_index = AD9986_FDDC_4_I,
            .virtual_converter5_index = AD9986_FDDC_4_Q,
            .virtual_converter6_index = AD9986_FDDC_5_I,
            .virtual_converter7_index = AD9986_FDDC_5_Q,
        },
        { 0 }
    };

    printf("AD9986: configuring JESD204C (uc_settings index 1)...\n");

    if (err = adi_ad9986_device_startup_tx(dev,
              4 /*main_interp*/, 4 /*chan_interp*/,
              dac_chan, main_shift, chan_shift, &jrx_param), err != API_CMS_ERROR_OK) {
        printf("AD9986: startup_tx (JRX/DAC path) failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JRX (DAC path) JESD204C configured.\n");

    if (err = adi_ad9986_device_startup_rx(dev,
              AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 |
              AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3  /*cddcs*/,
              AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 |
              AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5  /*fddcs*/,
              cddc_shift, fddc_shift,
              cddc_dcm, fddc_dcm,
              cc2r_en, fc2r_en,
              jtx_param, jesd_conv_sel), err != API_CMS_ERROR_OK) {
        printf("AD9986: startup_rx (JTX/ADC path) failed (%d).\n", err);
        return err;
    }
    printf("AD9986: JTX (ADC path) JESD204C configured.\n");

    printf("AD9986: JESD204C (index 1) configuration complete.\n");
    return API_CMS_ERROR_OK;
}

int main(int argc, char *argv[])
{
    int32_t err;
    int32_t readback;

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
            .sysref_mode = SYSREF_NONE,
        },
        .serdes_info = {
            .des_settings = {
                /* CTLE filter 1-4 (valid range); 2 = mid insertion-loss setting.
                 * Required for QUART_RATE (JESD204C > 16 Gbps) deserializer path. */
                .boost_mask  = 0xff,
                .ctle_filter = { 2, 2, 2, 2, 2, 2, 2, 2 },
                .lane_mapping = {
                    { 0, 1, 2, 3, 4, 5, 6, 7 }, /* link0: physical lane i → logical lane i */
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
            printf("  3 - Check AD9986 JESD link status\n");
            printf("  4 - Configure AD9986 JESD204C (UC1 params, 7864.32 MHz clock)\n");
            printf("  5 - Check HMC7044 CH_13 SYSREF status\n");
            printf("  6 - Unmute HMC7044 CH_13 SYSREF output (clear force_mute)\n");
            printf("  7 - Exit\n");
            printf("Enter choice: ");
            if (scanf("%d", &user_choice) != 1) {
                /* flush invalid input or exit on EOF */
                int c;
                if (feof(stdin))
                    break;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("Invalid input. Please enter 1-7.\n");
                continue;
            }
            if (user_choice == 1) {
                err = app_jesd_ip_reg_verify();
            } else if (user_choice == 2) {
                app_jesd_ip_reg_read_test();
            } else if (user_choice == 3) {
                app_ad9986_link_status_check(&ad9986_dev);
            } else if (user_choice == 4) {
                app_ad9986_jesd204c_config(&ad9986_dev);
            } else if (user_choice == 5) {
                app_hmc7044_ch13_status(&hmc7044_dev);
            } else if (user_choice == 6) {
                app_hmc7044_ch13_unmute(&hmc7044_dev);
            } else if (user_choice == 7) {
                break;
            } else {
                printf("Invalid choice. Please enter 1-7.\n");
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
    if (err = app_ad9986_prbs_test(&ad9986_dev, PRBS7), err != API_CMS_ERROR_OK) {
        goto cleanup;
    }

    /* The full JESD link bring-up is available through the reused adi_ads9_*
     * API, e.g. adi_ads9_config_jesd(jrx_param, jtx_param) followed by the
     * AD9986 datapath startup. Wire in the desired use-case parameters here.
     *
     * After JESD link setup, verify the SerDes PLL before enabling links:
     *     if (err = app_ad9986_jesd_pll_lock_check(&ad9986_dev), ...) goto cleanup;
     */

    printf("Control-plane bring-up complete.\n");

cleanup:
    lattice_user_data_free(&ad9986_dev.hal_info.user_data);
    lattice_user_data_free(&hmc7044_dev.hal_info.user_data);
    lattice_hw_close();
    return err;
}
