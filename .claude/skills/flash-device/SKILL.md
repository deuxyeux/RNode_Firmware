---
name: flash-device
description: Use when building firmware for a specific RNode board and flashing it to a connected device via arduino-cli/Makefile, including the post-flash step that clears the "Firmware Corrupt" state. Triggers on "flash the device", "upload firmware", "build and flash", or when a board-specific firmware change needs to be tested on real hardware.
---

# Flashing an RNode device

Flashing is a destructive, hard-to-reverse action on real hardware — confirm the target board and port with the user before running `make upload-*` unless they've already asked for it explicitly in this turn.

## 1. Build the target board LAST

`arduino-cli upload` (and the Makefile's `upload-*` targets) flash whatever was **most recently compiled** — they don't necessarily rebuild first. If you compiled a different board after the one you want to flash, `make upload-<board>` will silently flash the wrong firmware.

- Either run `make firmware-<board>` immediately before `make upload-<board>`, with no other `firmware-*` build in between, or
- Pass `--input-dir` pointing at the specific build output to `arduino-cli upload` directly.

Board names come from the `firmware-*` / `upload-*` targets in `Makefile` (e.g. `heltec_t096`, `heltec_t114`, `tbeam`, `rak4631`, `t3s3`, `techo`, ...). Run `grep '^upload-' Makefile` to list them.

## 2. Flash

```
make upload-<board>
```

This runs `arduino-cli upload -p /dev/ttyACM0 --fqbn <fqbn>`, then normally chains into `rnodeconf ... --firmware-hash $(./partition_hashes from_device ...)` to set the target firmware hash. That chained step requires the `RNS` Python module (`import RNS`) to be importable in the active shell — if it's not installed/activated, this step fails with `ModuleNotFoundError: No module named 'RNS'` even though the flash itself succeeded. Don't treat that failure as a flash failure; move to step 3.

`rnodeconf` also doesn't know every board (e.g. the Heltec T096) — for those the chained hash step fails or errors out on `-H/--firmware-hash` even with RNS available.

## 3. Clear "Firmware Corrupt" by setting the firmware hash

Whenever the chained `rnodeconf --firmware-hash` step didn't succeed (missing `RNS`, or a board `rnodeconf` doesn't support), run the standalone script instead:

```
~/Development/RNode_Scripts/set-hash-device [port]   # defaults to /dev/ttyACM0
```

This talks raw KISS to the device (same commands as the RNode_Flasher web tool) to read the actual firmware hash and write it back as the target hash, independent of `rnodeconf`'s board database. It works for any board, including ones `rnodeconf` doesn't recognize. Expect ~15-20s — persisting the hash blocks the device and may drop/re-enumerate the CDC port; the script retries automatically.

**Always run this after a successful flash**, even if the chained `rnodeconf` step in `make upload-*` appeared to work — if it errored for any reason, the device will otherwise report "Firmware Corrupt" until the hash is set.

## Quick reference

```
make firmware-<board>          # build (do this last before upload)
make upload-<board>            # flash over USB serial, port defaults to /dev/ttyACM0
~/Development/RNode_Scripts/set-hash-device [port]   # fix up target hash if step above's rnodeconf call failed
```
