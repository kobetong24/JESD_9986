#!/usr/bin/env bash
#
# build_rpi.sh - cross-compile the AD9986 RPi/Lattice control-plane app for
#                Raspberry Pi 5 (aarch64), or build natively when run on the Pi.
#
# Usage:
#   ./build_rpi.sh                 # cross-compile (release) with aarch64-linux-gnu-
#   CONFIG=debug ./build_rpi.sh    # cross-compile the debug (gcov) configuration
#   CROSS_COMPILE= ./build_rpi.sh  # native build (run this ON the Raspberry Pi)
#
set -euo pipefail

CROSS_COMPILE="${CROSS_COMPILE-aarch64-linux-gnu-}"
CONFIG="${CONFIG:-release}"
CC_BIN="${CROSS_COMPILE}gcc"

cd "$(dirname "$0")"

if [ -n "${CROSS_COMPILE}" ] && ! command -v "${CC_BIN}" >/dev/null 2>&1; then
    echo "error: cross-compiler '${CC_BIN}' not found."
    echo "Install it on a Debian/Ubuntu host with:"
    echo "    sudo apt-get update && sudo apt-get install -y gcc-aarch64-linux-gnu"
    echo "or run this script directly on the Raspberry Pi with: CROSS_COMPILE= ./build_rpi.sh"
    exit 1
fi

echo ">> make CONFIG=${CONFIG} CROSS_COMPILE='${CROSS_COMPILE}'"
make CONFIG="${CONFIG}" CROSS_COMPILE="${CROSS_COMPILE}" clean >/dev/null 2>&1 || true
make CONFIG="${CONFIG}" CROSS_COMPILE="${CROSS_COMPILE}"

BIN="${CONFIG}/ad9986_rpi_app"
echo ""
echo ">> Output: ${BIN}"
file "${BIN}"

if file "${BIN}" | grep -q "aarch64"; then
    echo ">> OK: aarch64 binary, compatible with Raspberry Pi 5."
elif [ -z "${CROSS_COMPILE}" ]; then
    echo ">> Built natively ($(uname -m))."
else
    echo ">> WARNING: output is not aarch64 - check the toolchain prefix."
fi
