"""
Automated BLE pair -> disconnect -> reconnect regression test for RNode.

Drives the exact cycle that's been failing manually: arm the RNode's
on-device pairing window over USB/KISS, pair fresh over BLE, confirm KISS
traffic works, disconnect, reconnect *without* re-pairing (this is what the
flasher/any normal second connection does), and confirm KISS traffic still
works. Prints a clear PASS/FAIL per stage instead of requiring a btmon
capture to interpret.

Usage:
    pip install bleak pyserial
    python ble_reconnect_test.py [--serial PORT] [--address MAC] [--skip-arm]

    --serial PORT    Serial port for the CMD_BT_CTRL pairing-mode-arm step
                      (default /dev/ttyACM0). Skipped if --address is given
                      together with --skip-arm.
    --address MAC    Connect directly instead of scanning.
    --skip-arm       Don't send the enable-pairing KISS command - use this
                      if the RNode is already bonded and you only want to
                      test the reconnect (stage 2) behavior.
"""

import argparse
import asyncio
import sys
import time

import serial
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

FEND = 0xC0
CMD_DETECT = 0x08
CMD_BT_CTRL = 0x46
DETECT_REQ = 0x73
DETECT_RESP = 0x46

DETECT_FRAME = bytes([FEND, CMD_DETECT, DETECT_REQ, FEND])
ENABLE_PAIRING_FRAME = bytes([FEND, CMD_BT_CTRL, 0x02, FEND])


def pass_fail(ok):
    return "PASS" if ok else "FAIL"


def arm_pairing_mode(port):
    print(f"[arm] opening {port} and sending enable-pairing KISS frame...")
    with serial.Serial(port, 115200, timeout=2) as s:
        time.sleep(0.5)
        s.write(ENABLE_PAIRING_FRAME)
        s.flush()
        time.sleep(0.5)
    print("[arm] sent - RNode should be in pairing mode for ~35s")


async def _scan_tolerant(timeout):
    """BleakScanner.discover()'s own __aexit__ can raise
    'org.bluez.Error.Failed: No discovery started' on this BlueZ/bleak combo
    (a session race with whatever else is also poking StartDiscovery/
    StopDiscovery on the same adapter - KDE's applet, a browser's Web
    Bluetooth backend, etc.), which throws away the devices it already
    found. Drive start()/stop() manually and swallow that specific error on
    teardown instead of losing the scan results.
    """
    scanner = BleakScanner()
    await scanner.start()
    await asyncio.sleep(timeout)
    try:
        await scanner.stop()
    except Exception as e:
        print(f"[scan] (ignoring scanner.stop() error: {e!r})")
    return scanner.discovered_devices


async def find_rnode(timeout=10.0):
    print(f"[scan] scanning {timeout:.0f}s for RNode UART service...")
    devices = await _scan_tolerant(timeout)
    devices = [d for d in devices if d.name and d.name.startswith("RNode")]
    for d in devices:
        print(f"[scan]   found: {d.name!r}  {d.address}")
    return devices[0].address if devices else None


async def kiss_roundtrip(client, label, timeout=5.0):
    """Write CMD_DETECT, wait for DETECT_RESP on notify. Returns True/False."""
    got_resp = asyncio.Event()

    def on_notify(_handle, data: bytearray):
        print(f"[{label}] << notify: {data.hex(' ')}")
        if DETECT_RESP in data and CMD_DETECT in data:
            got_resp.set()

    await client.start_notify(TX_UUID, on_notify)
    try:
        print(f"[{label}] >> writing CMD_DETECT")
        await client.write_gatt_char(RX_UUID, DETECT_FRAME, response=True)
    except Exception as e:
        print(f"[{label}] write failed: {e!r}")
        await client.stop_notify(TX_UUID)
        return False

    try:
        await asyncio.wait_for(got_resp.wait(), timeout=timeout)
        ok = True
    except asyncio.TimeoutError:
        print(f"[{label}] timed out waiting for DETECT_RESP")
        ok = False

    await client.stop_notify(TX_UUID)
    return ok


async def connect_and_test(address, label, do_pair):
    print(f"\n=== {label}: connecting to {address} ===")
    results = {"connect": False, "pair": None, "kiss": False}
    try:
        async with BleakClient(address, timeout=15.0) as client:
            results["connect"] = client.is_connected
            print(f"[{label}] connected: {client.is_connected}")

            if do_pair:
                try:
                    paired = await client.pair()
                    results["pair"] = bool(paired)
                    print(f"[{label}] pair() result: {paired}")
                except Exception as e:
                    print(f"[{label}] pair() raised: {e!r}")
                    results["pair"] = False

            # give the link a moment to settle (encryption/bond resume)
            await asyncio.sleep(1.5)

            results["kiss"] = await kiss_roundtrip(client, label)

            print(f"[{label}] disconnecting...")
    except Exception as e:
        print(f"[{label}] connection failed: {e!r}")

    return results


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--serial", default="/dev/ttyACM0")
    ap.add_argument("--address", default=None)
    ap.add_argument("--skip-arm", action="store_true")
    args = ap.parse_args()

    if not args.skip_arm:
        arm_pairing_mode(args.serial)
        await asyncio.sleep(1.0)

    address = args.address
    if address is None:
        address = await find_rnode()
        if address is None:
            print("No RNode found in scan. Pass --address AA:BB:CC:DD:EE:FF instead.")
            sys.exit(1)

    stage1 = await connect_and_test(address, "stage1-fresh-pair", do_pair=not args.skip_arm)

    print("\n[main] waiting 3s before reconnect (simulates closing/reopening the flasher)...")
    await asyncio.sleep(3.0)

    stage2 = await connect_and_test(address, "stage2-reconnect", do_pair=False)

    print("\n================ SUMMARY ================")
    print(f"stage1 (fresh pair)  connect={pass_fail(stage1['connect'])}  "
          f"pair={stage1['pair']}  kiss_roundtrip={pass_fail(stage1['kiss'])}")
    print(f"stage2 (reconnect)   connect={pass_fail(stage2['connect'])}  "
          f"kiss_roundtrip={pass_fail(stage2['kiss'])}")
    print("===========================================")

    if stage1["connect"] and stage1["kiss"] and stage2["connect"] and stage2["kiss"]:
        print("RESULT: PASS - reconnect survived, KISS traffic worked both times.")
        sys.exit(0)
    else:
        print("RESULT: FAIL - see stage detail above.")
        sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())
