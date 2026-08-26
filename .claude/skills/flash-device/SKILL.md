---
name: flash-device
description: Use when building firmware for a specific RNode board and flashing it to a connected device via PlatformIO, including the post-flash step that clears the "Firmware Corrupt" state. Triggers on "flash the device", "upload firmware", "build and flash", or when a board-specific firmware change needs to be tested on real hardware.
---

# Flashing an RNode device

Flashing is a destructive, hard-to-reverse action on real hardware — confirm the target board and port with the user before running `pio run -t upload` unless they've already asked for it explicitly in this turn.

## 1. Build and flash with PlatformIO

```
pio run -e <env> -t upload
```

This builds and uploads in one step, to each environment's own isolated `.pio_build/<env>/` directory — unlike the old arduino-cli/Makefile flow, there's no stale-last-compiled-target footgun, so no need to build a throwaway target out of order first.

Env names come from `platformio.ini` (e.g. `heltec_t096`, `heltec_t114`, `tbeam`, `rak4631`, `t3s3`, `techo`, ...). Run `grep '^\[env:' platformio.ini` to list them. Names mostly match the Makefile's `firmware-*`/`upload-*` board names, with a few exceptions (e.g. Makefile's `tbeam_sx126x` is `tbeam_sx1262` in platformio.ini) — check both if a name doesn't resolve.

Upload port defaults to auto-detect (confirmed working via `/dev/ttyACM0` on nRF52 boards); pass `--upload-port <port>` to pio if it picks the wrong one.

### If the board has no platformio.ini env yet

A few boards (extled variants, `mega2560`) are still Makefile/arduino-cli only. For those, fall back to:

```
make firmware-<board>          # build (do this last before upload — arduino-cli upload flashes whatever was most recently compiled)
make upload-<board>            # flash over USB serial, port defaults to /dev/ttyACM0
```

`make upload-<board>` normally chains into `rnodeconf ... --firmware-hash ...`, which needs the `RNS` Python module importable in the active shell and doesn't know every board (e.g. Heltec T096) — don't treat a failure there as a flash failure, just move to step 2 below.

## 2. Clear "Firmware Corrupt" by setting the firmware hash

`pio run -t upload` does **not** set the target firmware hash — always run this after every PlatformIO flash:

```
~/Development/RNode_Scripts/set-hash-device [port]   # defaults to /dev/ttyACM0
```

This talks raw KISS to the device (same commands as the RNode_Flasher web tool) to read the actual firmware hash and write it back as the target hash, independent of any board database. It works for any board. Expect ~15-20s — persisting the hash blocks the device and may drop/re-enumerate the CDC port; the script retries automatically. A "no response" readback attempt or two during the retry is normal, not a failure.

(For the Makefile fallback path: also always run this, even if the chained `rnodeconf` step in `make upload-*` appeared to work — if it errored for any reason, the device will otherwise report "Firmware Corrupt" until the hash is set.)

## Quick reference

```
pio run -e <env> -t upload                           # build + flash in one step (preferred)
~/Development/RNode_Scripts/set-hash-device [port]    # always run after, clears "Firmware Corrupt"
```
