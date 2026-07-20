#!/usr/bin/env python3
"""
run_with_logic2.py  --  Execute ad9986_rpi_app while capturing JESD IP SPI
                        signals (SPI0 CS0) with the Saleae Logic Pro 8 via
                        Logic 2 automation API.

Channel mapping (user-supplied)
--------------------------------
    ch0  CLK   -- SPI0_SCLK  (GPIO11, RPi5 pin 23)
    ch1  MOSI  -- SPI0_MOSI  (GPIO10, RPi5 pin 19)
    ch2  CS    -- SPI0_CE0_N (GPIO8,  RPi5 pin 24)  active-low
    ch3  MISO  -- SPI0_MISO  (GPIO9,  RPi5 pin 21)

SPI parameters (JESD IP / SPI0 CS0, from lattice.c)
-----------------------------------------------------
    Mode   : SPI Mode 0  (CPOL=0 CPHA=0)
    Clock  : 10 MHz  (LATTICE_SPI_JESD_HZ)
    Order  : MSB first
    Frame  : 6 bytes  [CMD][ADDR8][D3][D2][D1][D0]
             CMD 0x02 = write,  CMD 0x03 = read
"""

import csv as _csv
import os
import subprocess
import sys
import time
from pathlib import Path

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
LOGIC2_APPIMAGE  = Path("/home/admin/Logic-2.4.45-linux-arm64.AppImage")
APP_DIR          = Path(__file__).resolve().parent
APP_BIN          = APP_DIR / "release" / "ad9986_rpi_app"
CAPTURE_DIR      = APP_DIR / "captures"

# Saleae channels (0-based) -- user mapping
CH_CLK   = 0   # SPI0_SCLK
CH_MOSI  = 1   # SPI0_MOSI
CH_CS    = 2   # SPI0_CE0_N  (active-low)
CH_MISO  = 3   # SPI0_MISO

DIGITAL_SAMPLE_RATE  = 50_000_000   # 50 MHz -- 5x oversampling for 10 MHz SPI
DIGITAL_THRESHOLD_V  = 3.3
CAPTURE_BUFFER_MB    = 64

LOGIC2_GRPC_PORT         = 10430
LOGIC2_CONNECT_TIMEOUT   = 30.0


# ---------------------------------------------------------------------------
# SPI frame decoder (runs on the raw digital CSV after capture)
# ---------------------------------------------------------------------------
def decode_spi_frames(csv_path: str):
    """
    Decode SPI Mode 0 (CPOL=0, CPHA=0) frames from a raw-digital CSV.

    Each row in the CSV has columns:
        Time [s], Channel <CH_CLK>, Channel <CH_MOSI>, Channel <CH_CS>, Channel <CH_MISO>

    Returns a list of dicts:
        { 't_start', 't_end', 'mosi': [byte, ...], 'miso': [byte, ...] }
    """
    col_clk  = f"Channel {CH_CLK}"
    col_mosi = f"Channel {CH_MOSI}"
    col_cs   = f"Channel {CH_CS}"
    col_miso = f"Channel {CH_MISO}"

    rows = []
    with open(csv_path) as f:
        for row in _csv.DictReader(f):
            rows.append({
                't':    float(row["Time [s]"]),
                'clk':  int(row[col_clk]),
                'mosi': int(row[col_mosi]),
                'cs':   int(row[col_cs]),
                'miso': int(row[col_miso]),
            })

    if not rows:
        return []

    frames = []
    i = 0
    n = len(rows)

    while i < n:
        # Find CS falling edge (1->0)
        while i < n and not (rows[i-1]['cs'] == 1 and rows[i]['cs'] == 0):
            i += 1
        if i >= n:
            break

        frame_start = rows[i]['t']
        mosi_bits, miso_bits = [], []
        prev_clk = 0

        # Collect bits until CS rises
        while i < n and rows[i]['cs'] == 0:
            cur_clk = rows[i]['clk']
            # Mode 0: sample on rising CLK edge
            if prev_clk == 0 and cur_clk == 1:
                mosi_bits.append(rows[i]['mosi'])
                miso_bits.append(rows[i]['miso'])
            prev_clk = cur_clk
            i += 1

        frame_end = rows[i]['t'] if i < n else rows[-1]['t']

        # Pack bits into bytes (MSB first)
        def bits_to_bytes(bits):
            result = []
            for b in range(0, len(bits) - 7, 8):
                val = 0
                for k in range(8):
                    val = (val << 1) | bits[b + k]
                result.append(val)
            return result

        frames.append({
            't_start': frame_start,
            't_end':   frame_end,
            'mosi':    bits_to_bytes(mosi_bits),
            'miso':    bits_to_bytes(miso_bits),
        })

    return frames


def print_jesd_frames(frames):
    """Pretty-print decoded JESD IP frames with CMD / ADDR / DATA interpretation."""
    CMD_NAMES = {0x02: "WRITE", 0x03: "READ"}

    print()
    print("  ── JESD IP SPI0 CS0 decoded frames ─────────────────────────────")
    if not frames:
        print("    (no frames captured -- check CS channel wiring)")
        return

    for idx, f in enumerate(frames):
        mo = f['mosi']
        mi = f['miso']
        t_ms = f['t_start'] * 1e3
        dur_us = (f['t_end'] - f['t_start']) * 1e6

        mo_hex = " ".join(f"{b:02X}" for b in mo)
        mi_hex = " ".join(f"{b:02X}" for b in mi)

        print(f"\n  Frame {idx}  t={t_ms:.3f} ms  ({len(mo)} bytes  {dur_us:.1f} us)")
        print(f"    MOSI: {mo_hex}")
        print(f"    MISO: {mi_hex}")

        if len(mo) < 2:
            print("    (frame too short to decode)")
            continue

        cmd  = mo[0]
        addr = mo[1]
        cmd_name = CMD_NAMES.get(cmd, f"CMD=0x{cmd:02X}")

        if cmd == 0x02 and len(mo) >= 6:   # WRITE
            data_mosi = (mo[2] << 24) | (mo[3] << 16) | (mo[4] << 8) | mo[5]
            print(f"    >> {cmd_name}  addr=0x{addr:02X}  data=0x{data_mosi:08X}")

        elif cmd == 0x03 and len(mi) >= 6:  # READ
            data_miso = (mi[2] << 24) | (mi[3] << 16) | (mi[4] << 8) | mi[5]
            print(f"    >> {cmd_name}  addr=0x{addr:02X}  data=0x{data_miso:08X}  (from MISO)")
            # Also show MISO raw for all bytes to aid timing diagnosis
            print(f"    MISO all bytes: {mi_hex}")

        else:
            print(f"    >> {cmd_name}  addr=0x{addr:02X}  (unexpected length {len(mo)})")

    print()


# ---------------------------------------------------------------------------
def main() -> int:
    try:
        from saleae.automation import (
            Manager,
            LogicDeviceConfiguration,
            CaptureConfiguration,
            DigitalTriggerCaptureMode,
            DigitalTriggerType,
        )
    except ImportError:
        print("ERROR: saleae automation library not found.")
        print("       Install: pip3 install --break-system-packages logic2-automation")
        return 1

    if not LOGIC2_APPIMAGE.exists():
        print(f"ERROR: Logic 2 AppImage not found at {LOGIC2_APPIMAGE}")
        return 1

    if not APP_BIN.exists():
        print(f"ERROR: binary not found at {APP_BIN}  (run build first)")
        return 1

    CAPTURE_DIR.mkdir(parents=True, exist_ok=True)
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    sal_path  = str(CAPTURE_DIR / f"jesd_spi_{timestamp}.sal")

    print("=" * 60)
    print("JESD IP SPI0 CS0  +  Logic 2 capture / decode")
    print("=" * 60)
    print(f"  Channel map : CLK={CH_CLK}  MOSI={CH_MOSI}  CS={CH_CS}  MISO={CH_MISO}")
    print(f"  Sample rate : {DIGITAL_SAMPLE_RATE // 1_000_000} MHz digital")
    print(f"  Threshold   : {DIGITAL_THRESHOLD_V} V")
    print(f"  Trigger     : CS (ch{CH_CS}) falling edge")
    print()

    print("Step 1/5  Launching Logic 2 ...")
    with Manager.launch(
        application_path=str(LOGIC2_APPIMAGE),
        port=LOGIC2_GRPC_PORT,
        connect_timeout_seconds=LOGIC2_CONNECT_TIMEOUT,
    ) as mgr:
        app_info = mgr.get_app_info()
        print(f"          Logic 2 v{app_info.app_version} connected")

        print("Step 2/5  Detecting Saleae device ...")
        devices = mgr.get_devices(include_simulation_devices=False)
        if not devices:
            print("ERROR: no Saleae device found.")
            return 1
        device = devices[0]
        print(f"          Found {device.device_type.name}  id={device.device_id}")

        print("Step 3/5  Starting capture (trigger: CS falling edge) ...")
        device_cfg = LogicDeviceConfiguration(
            enabled_digital_channels=[CH_CLK, CH_MOSI, CH_CS, CH_MISO],
            digital_sample_rate=DIGITAL_SAMPLE_RATE,
            digital_threshold_volts=DIGITAL_THRESHOLD_V,
        )
        capture_cfg = CaptureConfiguration(
            buffer_size_megabytes=CAPTURE_BUFFER_MB,
            capture_mode=DigitalTriggerCaptureMode(
                trigger_type=DigitalTriggerType.FALLING,
                trigger_channel_index=CH_CS,
                after_trigger_seconds=1.0,
            ),
        )
        capture = mgr.start_capture(
            device_id=device.device_id,
            device_configuration=device_cfg,
            capture_configuration=capture_cfg,
        )
        print("          Armed -- waiting for CS trigger ...")

        print("Step 4/5  Running ad9986_rpi_app ...")
        print()
        result = subprocess.run(['sudo', str(APP_BIN)], cwd=str(APP_DIR), check=False)
        print()
        if result.returncode != 0:
            print(f"WARNING: app exited with code {result.returncode}")
        else:
            print("          Application completed successfully.")

        print("Step 5/5  Waiting for post-trigger window ...")
        capture.wait()
        print("          Capture complete.")

        raw_dir = str(CAPTURE_DIR / "raw_digital")
        capture.export_raw_data_csv(
            directory=raw_dir,
            digital_channels=[CH_CLK, CH_MOSI, CH_CS, CH_MISO],
        )
        csv_path = f"{raw_dir}/digital.csv"
        print(f"          Raw CSV -> {csv_path}")

        # Decode and print every JESD IP SPI frame
        frames = decode_spi_frames(csv_path)
        print_jesd_frames(frames)

        capture.save_capture(filepath=sal_path)
        print(f"          Logic 2 capture -> {sal_path}")
        capture.close()

    print("Done.")
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
