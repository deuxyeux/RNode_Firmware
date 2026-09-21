/*
 * bz2 decompression for received Resource payloads.
 *
 * Real Reticulum's Resource class (Python) bz2-compresses anything sent
 * via a Link/DIRECT-method transfer by default (Resource.py's own
 * auto_compress option, on unless the sender opts out) - a real Nomadnet
 * client sending an LXMF message larger than a single packet's worth
 * (DIRECT method, used automatically once OPPORTUNISTIC's single-packet
 * size is exceeded) goes through this path unconditionally. This C++
 * port's own Resource.cpp never produces compressed output itself
 * (_auto_compress is forced false regardless of the auto_compress()
 * setter - see Resource.cpp), so this is decompression-only: this
 * device never needs to *create* a compressed Resource, only decode one
 * a real peer sent it.
 *
 * Wraps the vendored, decompression-only port of libbzip2 1.0.8 in
 * bzip2/ (upstream Julian Seward, BSD-style license, see bzip2/LICENSE) -
 * the exact same code Python's own bz2 module wraps, so wire compatibility
 * is guaranteed rather than approximated.
 */

#pragma once

#include "../Bytes.h"

#include <cstddef>

namespace RNS {

	// Decompresses `compressed` (a full bz2 stream, as produced by
	// Python's bz2.compress()) into `out`. `expected_size` must be the
	// exact decompressed size, known upfront from the resource
	// advertisement's uncompressed-size field (ResourceAdvertisement::_d,
	// Resource.cpp's Resource::accept()) - bzip2's buffer-to-buffer API
	// needs a correctly-sized destination, it does not grow one. Returns
	// false without allocating anything if expected_size exceeds
	// max_size (a DoS guard against a peer advertising a small
	// compressed blob that claims to expand to something enormous) or
	// on any decompression error (malformed stream, CRC mismatch,
	// internal consistency failure).
	bool bz2_decompress(const Bytes& compressed, size_t expected_size, Bytes& out, size_t max_size);

}
