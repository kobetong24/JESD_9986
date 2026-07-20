# Raspberry Pi 5 + Lattice FPGA Platform (SPI Control Plane)

## Overview

This platform ports the AD9986 control/configuration interface to a
**Raspberry Pi 5** acting as the SPI host, with a **Lattice FPGA** serving as a
proxy/bridge to the JESD IP and the downstream devices.

### SPI device mapping

| Bus | Chip select | spidev node (default) | Target |
|-----|-------------|-----------------------|--------|
| SPI0 | CS0 | `/dev/spidev0.0` | JESD IP register interface (inside the Lattice FPGA) |
| SPI0 | CS1 | `/dev/spidev0.1` | AD9986 (proxied by the Lattice FPGA) |
| SPI1 | CS0 | `/dev/spidev1.0` | HMC7044 clock generator |

SPI access uses the standard Linux **spidev** driver (`SPI_IOC_MESSAGE`). The
AD9986 `RESETB` pin is driven through the Linux **GPIO character device**
(GPIO v2 ioctls). No external libraries are required.

### Key design point: JESD IP register access over SPI

On the ADS9 platform the JESD IP registers were memory-mapped, accessed through
`ads9_axi_reg_read32()` / `ads9_axi_reg_write32()`. On this platform those two
functions are **re-implemented over SPI0 CS0**, so the entire existing
`adi_ads9_*` JESD configuration API (`adi_ads9_config_jesd`, `adi_ads9_ver_get`,
link-status, clock selects, and so on) is **reused unchanged** — it simply
reaches the FPGA over SPI instead of a memory map.

### Scope

This platform implements the **SPI control/configuration plane only**. The
JESD204 **DMA data-capture path** (the Xilinx CDMA engine used by the ADS9) is
**not supported**: the `cdma_*` entry points are provided as unsupported stubs
so `adi_ads9.c` links without the Xilinx DMA stack. The original ADS9 platform
remains untouched and buildable.

## Block diagram

```
   Raspberry Pi 5 (SPI host, Linux)
      |                                   |
      |  spidev0  (SPI0)                  |  spidev1 (SPI1)
      v                                   v
   Lattice FPGA                         HMC7044
      |  CS0 -> JESD IP (internal)      (clock generator)
      |  CS1 -> AD9986  -------> AD9986
      |  GPIO -> RESETB
```

## Files

| File | Purpose |
|------|---------|
| `platform/lattice/lattice.h` | Platform configuration (device nodes, SPI rates, JESD-IP frame format, GPIO) and HAL exports. |
| `platform/lattice/lattice.c` | spidev SPI HAL, JESD-IP AXI-over-SPI primitives, RESETB GPIO, delay, logging, per-device context, `cdma_*` stubs. |
| `app_rpi/ad9986_rpi_app.c` | Control-plane example: JESD IP version (via `adi_ads9`), AD9986 reset/identify, HMC7044 identify. |
| `app_rpi/Makefile`, `app_rpi/debug.mak` | Build configuration. Compiles `adi_ads9.c`; ADS9 directories are on the include path for headers only. |

## Configuration

All board-specific settings are `#define`s in `lattice.h` and can be overridden
from the build system, for example:

```
make CFLAGS_EXTRA='-DLATTICE_SPI_AD9986_DEV=\"/dev/spidev0.1\" -DLATTICE_RESETB_CHIP=\"/dev/gpiochip4\" -DLATTICE_RESETB_LINE=25'
```

| Macro | Default | Meaning |
|-------|---------|---------|
| `LATTICE_SPI_JESD_DEV` | `/dev/spidev0.0` | JESD IP register interface (SPI0 CS0). |
| `LATTICE_SPI_AD9986_DEV` | `/dev/spidev0.1` | AD9986 (SPI0 CS1). |
| `LATTICE_SPI_HMC7044_DEV` | `/dev/spidev1.0` | HMC7044 (SPI1 CS0). |
| `LATTICE_SPI_MODE` | `0` | SPI mode (CPOL=0, CPHA=0). |
| `LATTICE_SPI_BITS` | `8` | Bits per word. |
| `LATTICE_SPI_JESD_HZ` | `10000000` | JESD IP bus clock (Hz). |
| `LATTICE_SPI_AD9986_HZ` | `10000000` | AD9986 bus clock (Hz). |
| `LATTICE_SPI_HMC7044_HZ` | `5000000` | HMC7044 bus clock (Hz). |
| `LATTICE_SPI_HMC7044_3WIRE` | `1` | Open the HMC7044 bus (SPI1) in 3-wire `SPI_3WIRE` mode for shared-SDIO read-back. Set to `0` only for a 4-wire rework. |
| `LATTICE_RESETB_CHIP` | `/dev/gpiochip4` | GPIO chip carrying AD9986 `RESETB`. |
| `LATTICE_RESETB_LINE` | `25` | GPIO line offset for `RESETB`. |

### JESD IP SPI register frame format

The JESD IP register transaction over SPI0 CS0 is a fixed 6-byte, MSB-first
frame (configurable to match the Lattice register-bridge RTL):

```
  byte0 : [RW][A14..A8]      RW = 1 for read  (LATTICE_JESD_RW_READ = 0x80)
  byte1 : [A7..A0]
  byte2 : data[31:24]
  byte3 : data[23:16]
  byte4 : data[15:8]
  byte5 : data[7:0]
```

The register address passed to the AXI primitives is the **word** address (as
on the ADS9 memory map). If the Lattice bridge expects a **byte** address,
set `LATTICE_JESD_ADDR_SHIFT` to `2`. The read/write bit, address mask and
shift are all overridable `#define`s in `lattice.h`.

> **Confirm against your RTL.** This frame layout is the assumed protocol of the
> Lattice SPI-to-register bridge. Adjust the `LATTICE_JESD_*` macros so the
> software framing matches the FPGA design.

## Raspberry Pi 5 setup

1. **Enable both SPI controllers** with two chip selects on SPI0. In
   `/boot/firmware/config.txt`:

   ```
   dtparam=spi=on
   dtoverlay=spi0-2cs
   dtoverlay=spi1-1cs
   ```

   Reboot, then confirm the nodes:

   ```
   ls /dev/spidev0.0 /dev/spidev0.1 /dev/spidev1.0
   ```

2. **Permissions.** Add your user to the `spi` and `gpio` groups (or run with
   sufficient privileges) to open the spidev and gpiochip devices.

3. **RESETB GPIO.** On Raspberry Pi 5 the 40-pin header GPIOs are exposed by the
   RP1 chip, typically `/dev/gpiochip4`. Verify with `gpiodetect` / `gpioinfo`
   and set `LATTICE_RESETB_CHIP` / `LATTICE_RESETB_LINE` to match the wiring.

## HMC7044 3-wire SPI (this board)

On this board the HMC7044 SPI uses a single bidirectional data line
(`SPI1_SDIO` to `HMC_SDATA`, pin 20) through an auto-direction level
translator (U10FF). There is no separate MISO net for SPI1. Confirmed against
the schematic, the mapping is one-to-one: `SPI1_SCLK` to `SCLK` (pin 19),
`SPI1_SDIO` to `SDATA` (pin 20), `SPI1_CSB` to `SLEN` (pin 18).

Consequences for the software:

- The HMC7044 bus is opened in `SPI_3WIRE` mode by default
  (`LATTICE_SPI_HMC7044_3WIRE = 1`).
- Register writes clock all bytes out on the shared line.
- Register reads are performed as two half-duplex phases inside one chip-select
  assertion: a 2-byte instruction (write) phase followed by the data (read)
  phase, both issued in a single `SPI_IOC_MESSAGE`.
- The Raspberry Pi SPI controller must support `SPI_3WIRE`. If the kernel
  rejects it on a given platform, the alternative is a board rework to 4-wire
  (separate MISO, or `MOSI`/`MISO` bridged with a series resistor); in that case
  set `LATTICE_SPI_HMC7044_3WIRE = 0` to use standard full-duplex transfers.
- `HMC_GPIO1` (pin 31) routes to the FPGA, not to the Raspberry Pi, so a
  dedicated 4-wire SDO read-back path to the Pi is not available.

## Build

Native build on the Raspberry Pi 5:

```
cd src/ad9986_app/app_rpi
make
./debug/ad9986_rpi_app
```

Cross-compile for the Raspberry Pi 5 (aarch64) from a Linux host:

```
# one-time: install the cross toolchain
sudo apt-get update && sudo apt-get install -y gcc-aarch64-linux-gnu

cd src/ad9986_app/app_rpi
./build_rpi.sh                 # release build, aarch64, verifies output arch
# or explicitly:
make CONFIG=release CROSS_COMPILE=aarch64-linux-gnu-
```

This produces `release/ad9986_rpi_app`. Confirm it is for the Pi with:

```
file release/ad9986_rpi_app      # expect: ELF 64-bit LSB ... ARM aarch64
```

Copy it to the Pi and run it there:

```
scp release/ad9986_rpi_app <user>@<pi-host>:~
ssh <user>@<pi-host> 'sudo ./ad9986_rpi_app'
```

> A binary built with the host's native `gcc` (x86-64) will **not** run on the
> Raspberry Pi 5. Either cross-compile as above, or build natively on the Pi
> (`CROSS_COMPILE= ./build_rpi.sh`). The `CONFIG=release` build omits gcov
> instrumentation; `CONFIG=debug` keeps it for coverage/debugging.

## Expected output

```
AD9986 control-plane app (Raspberry Pi 5 + Lattice FPGA proxy)
  SPI0 CS0 -> JESD IP : /dev/spidev0.0
  SPI0 CS1 -> AD9986  : /dev/spidev0.1
  SPI1 CS0 -> HMC7044 : /dev/spidev1.0
lattice: spidev /dev/spidev0.0 opened (mode 0, 8 bits, 10000000 Hz).
lattice: spidev /dev/spidev0.1 opened (mode 0, 8 bits, 10000000 Hz).
lattice: spidev /dev/spidev1.0 opened (mode 0, 8 bits, 5000000 Hz).
JESD IP (FPGA) image vXX svYY
JESD IP reg[0x102] = 0x........
HMC7044 API vX.Y.Z
HMC7044 chip type 0x..., product id 0x...., revision 0x...
AD9986 API vX.Y.Z
AD9986 chip type 0x..., product id 0x...., revision 0x...
Control-plane bring-up complete.
```

A failed read on any line points to a wiring, chip-select, or device-node
mismatch on the corresponding bus.

## Extending to full JESD link bring-up

Because the `adi_ads9_*` API is reused over SPI, the JESD link can be configured
from `app_rpi` exactly as on the ADS9, for example
`adi_ads9_config_jesd(jrx_param, jtx_param)` together with the AD9986 datapath
startup (`adi_ad9986_device_startup_tx` / `_rx`) and the HMC7044 clock-tree
configuration (`adi_hmc7044_pll2_config_set`, `adi_hmc7044_output_config_set`,
`adi_hmc7044_reg_update`). Only the DMA data-capture step is unavailable on this
platform.
