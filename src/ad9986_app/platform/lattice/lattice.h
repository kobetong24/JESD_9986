/*!
 * @brief     Lattice FPGA / Raspberry Pi 5 platform configuration header file.
 *
 *            This platform targets a Raspberry Pi 5 acting as the SPI host.
 *            A Lattice FPGA is used as a proxy/bridge between the Raspberry Pi
 *            and the JESD IP and downstream devices.
 *
 *            SPI device mapping:
 *               - SPI0 CS0 (/dev/spidev0.0) : JESD IP register interface
 *                                             (inside the Lattice FPGA)
 *               - SPI0 CS1 (/dev/spidev0.1) : AD9986 (proxied by the FPGA)
 *               - SPI1 CS0 (/dev/spidev1.0) : HMC7044 clock generator
 *
 *            SPI access is performed through the standard Linux spidev driver
 *            (SPI_IOC_MESSAGE ioctls). The JESD IP register interface, which on
 *            the ADS9 platform was memory-mapped, is re-routed over SPI0 CS0 by
 *            re-implementing ads9_axi_reg_read32()/ads9_axi_reg_write32(); the
 *            existing adi_ads9_* JESD configuration API is reused unchanged.
 *            The AD9986 RESETB pin is driven through the Linux GPIO character
 *            device (GPIO v2 ioctls).
 *
 *            This platform implements the SPI control/configuration plane only.
 *            JESD204 DMA data capture (the Xilinx CDMA path) is not supported.
 *
 * @copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_LATTICE__
 * @{
 */
#ifndef __LATTICE_H__
#define __LATTICE_H__

/*============= I N C L U D E S ============*/
#include "adi_ad9986.h"
#include "adi_hmc7044.h"

/*============= D E F I N E S ==============*/
/*
 * The following defaults can be overridden from the build system
 * (e.g. -DLATTICE_SPI_JESD_DEV=\"/dev/spidev0.0\") to match the board
 * wiring without editing this file.
 */

/* SPI0 CS0 : JESD IP register interface (inside the Lattice FPGA) */
#ifndef LATTICE_SPI_JESD_DEV
#define LATTICE_SPI_JESD_DEV     "/dev/spidev0.0"
#endif

/* SPI0 CS1 : AD9986 (proxied by the Lattice FPGA) */
#ifndef LATTICE_SPI_AD9986_DEV
#define LATTICE_SPI_AD9986_DEV   "/dev/spidev0.1"
#endif

/* SPI1 CS0 : HMC7044 clock generator */
#ifndef LATTICE_SPI_HMC7044_DEV
#define LATTICE_SPI_HMC7044_DEV  "/dev/spidev1.0"
#endif

/* SPI mode for the AD9986 and HMC7044 buses */
#ifndef LATTICE_SPI_MODE
#define LATTICE_SPI_MODE         0
#endif

/* SPI mode for the JESD IP register bridge (SPI0 CS0).
 *
 * The Lattice FPGA bridge RTL drives MISO on the CLK rising edge (one
 * half-cycle later than standard Mode 0), so the host must sample on the
 * CLK falling edge: CPHA=1 → SPI Mode 1 (CPOL=0, CPHA=1). */
#ifndef LATTICE_SPI_JESD_MODE
#define LATTICE_SPI_JESD_MODE    1u
#endif

/* SPI bits per word */
#ifndef LATTICE_SPI_BITS
#define LATTICE_SPI_BITS         8
#endif

/*
 * HMC7044 3-wire SPI.
 *
 * On this board the HMC7044 SPI uses a single bidirectional data line
 * (SPI1_SDIO -> HMC_SDATA, pin 20) through an auto-direction level translator;
 * there is no separate MISO. Set this to 1 to open the HMC7044 spidev node in
 * SPI_3WIRE mode so register read-back works on the shared data line; register
 * reads are then performed as a write (instruction) phase followed by a read
 * (data) phase within a single chip-select assertion.
 *
 * Set to 0 only if the board is reworked for 4-wire operation (separate MISO,
 * or MOSI/MISO bridged), in which case standard full-duplex transfers are used.
 */
#ifndef LATTICE_SPI_HMC7044_3WIRE
#define LATTICE_SPI_HMC7044_3WIRE 0
#endif

/* SPI clock frequencies (Hz) */
#ifndef LATTICE_SPI_JESD_HZ
#define LATTICE_SPI_JESD_HZ      10000000u
#endif
/* AD9986 SPI clock is intentionally limited to 1 MHz.
 * The Lattice FPGA acts as a proxy between SPI0 CS1 and the AD9986 SPI
 * port.  At 10 MHz the FPGA MISO path introduces intermittent bit errors
 * on register readback (nondeterministic stuck bits during the API 8-bit
 * r/w access check), most likely due to combinatorial delay in the proxy
 * output path.  1 MHz provides sufficient hold-time margin.  For a
 * control-plane-only application the throughput reduction is immaterial. */
#ifndef LATTICE_SPI_AD9986_HZ
#define LATTICE_SPI_AD9986_HZ    1000000u
#endif
#ifndef LATTICE_SPI_HMC7044_HZ
#define LATTICE_SPI_HMC7044_HZ   5000000u
#endif

/* HMC7044 reference clock frequency on CLKIN0 (Hz).
 * Set to match the crystal or oscillator on the board's CLKIN0 input.
 * Default 122.88 MHz -- the JESD204 fractional-N standard; other common
 * choices are 100 MHz and 200 MHz.  Override with -DLATTICE_HMC7044_REF_CLK_HZ=<Hz>. */
#ifndef LATTICE_HMC7044_REF_CLK_HZ
#define LATTICE_HMC7044_REF_CLK_HZ   122880000ULL
#endif

/* AD9986 device clock frequency driven from HMC7044 CH_2 (Hz).
 * Must be achievable from the HMC7044 VCO by integer division.
 * 122.88 MHz: HMC7044 VCO = 22 x 122.88 = 2703.36 MHz,
 * CH_2 divider = 22, CH_6/CH_12 (122.88 MHz) dividers = 22.
 * Override with -DLATTICE_AD9986_DEV_CLK_HZ=<Hz>. */
#ifndef LATTICE_AD9986_DEV_CLK_HZ
#define LATTICE_AD9986_DEV_CLK_HZ    122880000ULL
#endif

/* AD9986 desired DAC sample rate (Hz).
 * adi_ad9986_device_clk_config_set() programs the on-chip PLL to derive
 * this rate from LATTICE_AD9986_DEV_CLK_HZ.  The PLL VCO must fall in
 * [5.8, 12] GHz (VCO = DAC_CLK * pll_div, pll_div = 1..4).
 * 7864.32 MHz = 64 x 122.88 MHz: ref_div=1 -> pfd=122.88 MHz,
 * pll_div=1, VCO=7.86432 GHz, n=8, m=8.  Valid range: 2.85..12.6 GHz. */
#ifndef LATTICE_AD9986_DAC_CLK_HZ
#define LATTICE_AD9986_DAC_CLK_HZ    7864320000ULL
#endif

/* AD9986 desired ADC sample rate (Hz).
 * Must be an integer sub-multiple of LATTICE_AD9986_DAC_CLK_HZ.
 * 3932.16 MHz = DAC clock / 2 = 32 x 122.88 MHz.
 * Valid range: 1.45..6 GHz. */
#ifndef LATTICE_AD9986_ADC_CLK_HZ
#define LATTICE_AD9986_ADC_CLK_HZ    3932160000ULL
#endif

/*
 * JESD IP SPI register frame format (SPI0 CS0).
 *
 * Each transaction is 6 bytes, MSB first:
 *
 *    byte0 : command  (0x02 = write, 0x03 = read)
 *    byte1 : [A7..A0]   8-bit register address
 *    byte2 : data[31:24]
 *    byte3 : data[23:16]
 *    byte4 : data[15:8]
 *    byte5 : data[7:0]
 *
 * For a read, the host clocks out dummy bytes in byte2..5.  Because the
 * FPGA bridge drives MISO on the CLK rising edge (SPI Mode 1), the kernel
 * driver (CPHA=1) samples on the falling edge and the four data bytes land
 * in rx[0..3] — one half-cycle earlier than in Mode 0.
 *
 * Adjust these macros to match the Lattice FPGA register-bridge RTL.
 */
#ifndef LATTICE_JESD_CMD_WRITE
#define LATTICE_JESD_CMD_WRITE   0x02u  /* command byte for register write */
#endif
#ifndef LATTICE_JESD_CMD_READ
#define LATTICE_JESD_CMD_READ    0x03u  /* command byte for register read  */
#endif
#ifndef LATTICE_JESD_ADDR_MASK
#define LATTICE_JESD_ADDR_MASK   0xFFu  /* 8-bit register address          */
#endif
#ifndef LATTICE_JESD_ADDR_SHIFT
#define LATTICE_JESD_ADDR_SHIFT  0      /* 0: word address, 2: byte addr   */
#endif

/*
 * GPIO character device and line offset used to drive the AD9986 RESETB pin.
 * On a Raspberry Pi 5 the 40-pin header GPIOs are usually exposed on
 * /dev/gpiochip4 (RP1). Set these to match the RESETB net wiring.
 */
#ifndef LATTICE_RESETB_CHIP
#define LATTICE_RESETB_CHIP      "/dev/gpiochip4"
#endif
#ifndef LATTICE_RESETB_LINE
#define LATTICE_RESETB_LINE      25u
#endif

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

int32_t lattice_hw_open(const char *log_file);
int32_t lattice_hw_close(void);
int32_t lattice_log_write(void *user_data, int32_t log_type, const char *comment, va_list argp);
int32_t lattice_spi_xfer_ad9986(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes);
int32_t lattice_spi_xfer_hmc7044(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes);
int32_t lattice_wait_us(void *user_data, uint32_t time_us);
int32_t lattice_hw_rst_pin_ctrl_ad9986(void *user_data, uint8_t pin_level);
int32_t lattice_user_data_create_ad9986(adi_ad9986_device_t *device, uint8_t spi_cs);
int32_t lattice_user_data_create_hmc7044(adi_hmc7044_device_t *device, uint8_t spi_cs);
int32_t lattice_user_data_free(void **user_data);

/*
 * JESD IP register access over SPI0 CS0. These provide the same symbols the
 * adi_ads9_* JESD API links against (declared in the ADS9 platform's ads9.h),
 * so that API is reused unchanged on the Lattice platform.
 */
int32_t ads9_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data);
int32_t ads9_axi_reg_write32(uint32_t reg_offset, uint32_t data);

#ifdef __cplusplus
}
#endif
#endif /*__LATTICE_H__*/
/*! @} */
