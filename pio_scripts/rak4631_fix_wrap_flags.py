Import("env")

# PlatformIO's own nordicnrf52 platform builder
# (builder/frameworks/arduino/adafruit.py) unconditionally adds
# -Wl,--wrap={malloc,free,realloc,calloc}, but the rakwireless:nrf52 core's
# own heap_3.c (version 1.3.3, currently the newest release available - see
# setup_rak4631_nrf52_pio.sh) only implements __wrap_malloc/__wrap_free -
# no __wrap_realloc/__wrap_calloc, unlike the newer heap_3.c bundled with
# Heltec_nRF52/promicro:nrf52 (diffed byte-for-byte: those two extra
# wrappers were added upstream after RAK's own last core release).
# arduino-cli's own platform.txt for this exact core only ever wraps
# malloc/free too (compiler.ldflags), so this restores parity with what a
# real arduino-cli build of this board actually links successfully -
# confirmed via a real "undefined reference to `__wrap_calloc`" link
# failure without this.
wrap_flags_to_remove = {"-Wl,--wrap=realloc", "-Wl,--wrap=calloc"}
env["LINKFLAGS"] = [f for f in env["LINKFLAGS"] if f not in wrap_flags_to_remove]
