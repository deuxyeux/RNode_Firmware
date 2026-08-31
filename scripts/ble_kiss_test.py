"""
Minimal BLE KISS probe for RNode, independent of MeshChatX / Reticulum.

Connects directly to the RNode's Nordic-UART-style GATT service (the same
UUIDs BLESerial.cpp/.h use), sends a KISS CMD_DETECT frame, and prints
whatever comes back on the TX (notify) characteristic. If this script gets
a DETECT_RESP back, the RNode, the pairing/bond, Python, bleak, and the
Windows BLE stack are all fine end-to-end - which would point the problem
squarely at MeshChatX's BLE handling. If this script also fails, the
problem is below the app layer (pairing/encryption/bleak/Windows stack)
rather than being MeshChatX-specific.

Usage:
    pip install bleak
    python ble_kiss_test.py                  # scan and pick first "RNode *"
    python ble_kiss_test.py AA:BB:CC:DD:EE:FF # connect directly by address
"""

import asyncio
import sys

from bleak import BleakClient, BleakScanner

# Same UUIDs as BLESerial.h
SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  # write (host -> RNode)
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  # notify (RNode -> host)

# KISS framing (Framing.h)
FEND = 0xC0
CMD_DETECT = 0x08
DETECT_REQ = 0x73
DETECT_RESP = 0x46
CMD_FW_VERSION = 0x50

DETECT_FRAME = bytes([FEND, CMD_DETECT, DETECT_REQ, FEND])
FW_VERSION_FRAME = bytes([FEND, CMD_FW_VERSION, 0x00, FEND])


async def find_rnode(timeout=10.0):
    print(f"Scanning for {timeout:.0f}s for a device advertising the RNode UART service...")
    devices = await BleakScanner.discover(timeout=timeout, service_uuids=[SERVICE_UUID])
    if not devices:
        # Fall back to a name-based scan in case the service UUID isn't in
        # the advertisement/scan-response bleak sees.
        print("  (nothing matched by service UUID, retrying with a plain scan filtered by name)")
        devices = [
            d for d in await BleakScanner.discover(timeout=timeout)
            if d.name and d.name.startswith("RNode")
        ]
    for d in devices:
        print(f"  found: {d.name!r}  {d.address}")
    return devices[0] if devices else None


def on_notify(_handle, data: bytearray):
    print(f"<< notify: {data.hex(' ')}")
    if DETECT_RESP in data and CMD_DETECT in data:
        print("   looks like a DETECT_RESP - RNode answered over BLE.")


async def main():
    if len(sys.argv) > 1:
        address = sys.argv[1]
    else:
        device = await find_rnode()
        if device is None:
            print("No RNode found in scan. Pass its MAC address as an argument instead, "
                  "e.g.: python ble_kiss_test.py AA:BB:CC:DD:EE:FF")
            return
        address = device.address

    print(f"\nConnecting to {address} ...")
    async with BleakClient(address) as client:
        print(f"Connected: {client.is_connected}")

        try:
            paired = await client.pair()
            print(f"Pair result: {paired}")
        except Exception as e:
            # Windows/bleak: if it's already OS-paired this can raise or
            # no-op depending on backend version - not fatal by itself.
            print(f"pair() raised (may be harmless if already paired): {e!r}")

        services = await client.get_services() if hasattr(client, "get_services") else client.services
        print(f"\nAll services Windows/bleak reports for this device ({len(list(services))}):")
        for s in services:
            print(f"  service {s.uuid}")
            for ch in s.characteristics:
                print(f"    char {ch.uuid}  props={ch.properties}")

        svc = services.get_service(SERVICE_UUID)
        if svc is None:
            print(f"\nService {SERVICE_UUID} not found on device - wrong device, or GATT discovery failed.")
            return
        print(f"Found service {SERVICE_UUID} with characteristics:")
        for ch in svc.characteristics:
            print(f"  {ch.uuid}  props={ch.properties}")

        await client.start_notify(TX_UUID, on_notify)

        print(f"\n>> writing CMD_DETECT frame: {DETECT_FRAME.hex(' ')}")
        try:
            await client.write_gatt_char(RX_UUID, DETECT_FRAME, response=True)
        except Exception as e:
            print(f"Write failed: {e!r}")
            print("If this is an authentication/access-denied style error, the GATT link "
                  "isn't encrypted/authenticated the way the RNode's firmware requires "
                  "(ESP_GATT_PERM_WRITE_ENC_MITM) - that's the bonding/encryption issue, "
                  "not a MeshChatX-specific one.")
            await asyncio.sleep(1)
            await client.stop_notify(TX_UUID)
            return

        await asyncio.sleep(2)

        print(f"\n>> writing CMD_FW_VERSION frame: {FW_VERSION_FRAME.hex(' ')}")
        await client.write_gatt_char(RX_UUID, FW_VERSION_FRAME, response=True)
        await asyncio.sleep(3)

        print(f"\nis_connected: {client.is_connected}")
        try:
            val = await client.read_gatt_char(TX_UUID)
            print(f"Direct read of TX characteristic value: {val.hex(' ') if val else '(empty)'}")
        except Exception as e:
            print(f"Direct read of TX failed: {e!r}")

        await asyncio.sleep(3)
        print(f"is_connected (after extra wait): {client.is_connected}")

        await client.stop_notify(TX_UUID)


if __name__ == "__main__":
    asyncio.run(main())
