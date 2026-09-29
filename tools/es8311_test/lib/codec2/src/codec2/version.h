// Hand-written stand-in for the file upstream Codec2's CMake build
// generates from cmake/version.h.in at configure time (CODEC2_VERSION_PATH,
// src/CMakeLists.txt) - this project builds via PlatformIO, not CMake, so
// there's no configure step to produce it. Values match the pinned
// upstream commit's project(CODEC2 VERSION 1.2.0) (CMakeLists.txt:15-16).
// Nothing in the vendored vocoder-core sources actually reads these
// macros - codec2.h just needs the file to exist to satisfy its own
// #include <codec2/version.h>.

#ifndef CODEC2_HAVE_VERSION
#define CODEC2_HAVE_VERSION

#define CODEC2_VERSION_MAJOR 1
#define CODEC2_VERSION_MINOR 2
#define CODEC2_VERSION_PATCH 0
#define CODEC2_VERSION "1.2.0"

#endif  // CODEC2_HAVE_VERSION
