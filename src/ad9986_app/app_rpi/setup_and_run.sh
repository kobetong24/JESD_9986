#!/usr/bin/env bash
# setup_and_run.sh — Install dependencies, build, and run the AD9986 RPi app.
#
# Targets a fresh Raspberry Pi 5 running Raspberry Pi OS (Debian Bookworm/Bullseye).
# Run this script directly on the Raspberry Pi:
#
#   chmod +x setup_and_run.sh
#   ./setup_and_run.sh
#
# Optional environment variables:
#   CONFIG=debug   ./setup_and_run.sh   # build debug instead of release
#   SKIP_BUILD=1   ./setup_and_run.sh   # skip build, run existing binary

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONFIG="${CONFIG:-release}"
BINARY="${SCRIPT_DIR}/${CONFIG}/ad9986_rpi_app"

# ─── Colour helpers ────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()    { echo -e "${GREEN}[INFO]${NC}  $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*" >&2; }

# ─── 1. Prerequisite check: must run on aarch64 Linux ──────────────────────────
info "Platform: $(uname -m) / $(uname -s)"
if [[ "$(uname -m)" != "aarch64" ]]; then
    error "This script must run natively on the Raspberry Pi (aarch64)."
    error "For cross-compilation use build_rpi.sh on a host machine."
    exit 1
fi

# ─── 2. Install build dependencies ────────────────────────────────────────────
info "Checking build dependencies..."

MISSING_PKGS=()

check_pkg() {
    local pkg="$1"
    local bin="$2"
    if ! command -v "${bin}" >/dev/null 2>&1; then
        warn "  ${bin} not found — will install ${pkg}"
        MISSING_PKGS+=("${pkg}")
    else
        info "  ${bin} found ($(command -v "${bin}"))"
    fi
}

# gcc and make are the only build-time requirements.
# The platform code uses only standard Linux kernel headers (linux/spi/spidev.h,
# linux/gpio.h) which are bundled with linux-libc-dev / build-essential.
check_pkg "gcc"           "gcc"
check_pkg "make"          "make"
check_pkg "build-essential" "gcc"   # catches the full toolchain in one go

if [[ ${#MISSING_PKGS[@]} -gt 0 ]]; then
    info "Installing missing packages: ${MISSING_PKGS[*]}"
    sudo apt-get update -qq
    # Install build-essential (pulls gcc, make, libc-dev, linux-libc-dev)
    sudo apt-get install -y build-essential
    info "Build dependencies installed."
else
    info "All build dependencies present."
fi

# ─── 3. Verify Linux kernel interfaces are available ──────────────────────────
info "Checking required kernel interfaces..."

check_dev() {
    local dev="$1"
    if [[ -e "${dev}" ]]; then
        info "  ${dev} present"
    else
        warn "  ${dev} NOT found — check /boot/firmware/config.txt overlays"
    fi
}

# SPI0 (CS0 = JESD IP, CS1 = AD9986 proxy)
check_dev /dev/spidev0.0
check_dev /dev/spidev0.1
# SPI1 (CS0 = HMC7044)
check_dev /dev/spidev1.0
# GPIO character device (RP1 on Pi 5)
check_dev /dev/gpiochip4

# Advise the user if any SPI devices are missing
if [[ ! -e /dev/spidev0.0 || ! -e /dev/spidev0.1 || ! -e /dev/spidev1.0 ]]; then
    warn ""
    warn "One or more SPI devices are missing. Add these lines to"
    warn "/boot/firmware/config.txt and reboot:"
    warn "    dtparam=spi=on"
    warn "    dtoverlay=spi1-1cs"
    warn ""
fi

# ─── 4. Build ─────────────────────────────────────────────────────────────────
if [[ "${SKIP_BUILD:-0}" == "1" ]]; then
    warn "SKIP_BUILD=1 set — skipping build step."
else
    info "Building (CONFIG=${CONFIG})..."
    cd "${SCRIPT_DIR}"

    # Fix ownership of the output directory if it was previously built as root
    if [[ -d "${CONFIG}" ]]; then
        sudo chown -R "$(id -u):$(id -g)" "${CONFIG}" 2>/dev/null || true
    fi

    make CONFIG="${CONFIG}" CROSS_COMPILE= clean >/dev/null 2>&1 || true
    make CONFIG="${CONFIG}" CROSS_COMPILE=

    info "Build complete: ${BINARY}"
    file "${BINARY}"
fi

# ─── 5. Verify the binary exists before running ────────────────────────────────
if [[ ! -x "${BINARY}" ]]; then
    error "Binary not found or not executable: ${BINARY}"
    error "Run without SKIP_BUILD=1 to build first."
    exit 1
fi

# ─── 6. Execute ───────────────────────────────────────────────────────────────
info "Running ${BINARY} (requires sudo for SPI/GPIO access)..."
echo ""
sudo "${BINARY}"
