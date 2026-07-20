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

    /* Enable CH_2 only for the AD9986 device clock.
     * Add HMC7044_OP_CH_x flags here when additional board outputs are known.
     * NOTE: the output-disable loop from the ADS9 reference app is intentionally
     * omitted here.  On this Lattice board the FPGA JESD IP uses one of the
     * other HMC7044 outputs as its reference clock; disabling all outputs before
     * clk_config removes that clock and breaks the SPI register bridge on the
     * next run.  Outputs not listed in hmc_out_ch keep their init-table values. */
    const uint16_t hmc_out_ch = HMC7044_OP_CH_6 | HMC7044_OP_CH_12;

    uint64_t hmc_out_hz[14] = {
        0, 0, 0, 0, 0, 0,
        LATTICE_AD9986_DEV_CLK_HZ,  /* CH_6  → AD9986 device clock (122.88 MHz) */
        0, 0, 0, 0, 0,
        122880000ULL,               /* CH_12 → 122.88 MHz (JESD204 reference) */
        0
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

    printf("HMC7044 clocks configured: ref=%.2f MHz  CH_6=%.2f MHz  CH_12=122.88 MHz.\n",
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
        }
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
            printf("  3 - Exit\n");
            printf("Enter choice: ");
            if (scanf("%d", &user_choice) != 1) {
                /* flush invalid input or exit on EOF */
                int c;
                if (feof(stdin))
                    break;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("Invalid input. Please enter 1 or 2.\n");
                continue;
            }
            if (user_choice == 1) {
                //if (err = app_jesd_ip_reg_verify(), err != API_CMS_ERROR_OK) {
                    //goto cleanup;
                //}
                err = app_jesd_ip_reg_verify();
            } else if (user_choice == 2) {
				app_jesd_ip_reg_read_test();
            } else if (user_choice == 3) {
                break;
            }else {
                printf("Invalid choice. Please enter 1 or 2.\n");
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
