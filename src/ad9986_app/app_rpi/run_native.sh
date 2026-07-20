#!/usr/bin/env bash
# run_native.sh — clean, build (native), and execute the AD9986 RPi app.
# Run this directly on the Raspberry Pi 5.

set -euo pipefail

cd "$(dirname "$0")"

CONFIG="${CONFIG:-release}"

echo ">> Clean"
sudo make CONFIG="${CONFIG}" CROSS_COMPILE= clean

echo ""
echo ">> Build  (CONFIG=${CONFIG})"
make CONFIG="${CONFIG}" CROSS_COMPILE=

echo ""
echo ">> Run"
sudo "${CONFIG}/ad9986_rpi_app"
