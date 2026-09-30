#include "Bz2Decompress.h"

#include "../Log.h"

extern "C" {
	#include "bzip2/bzlib.h"
	#include <setjmp.h>
	// Defined in bzip2/bzlib_decompress.c - see that file's header
	// comment (point 2) for why AssertH()'s failure path longjmps here
	// instead of calling exit() like upstream bzlib does.
	extern jmp_buf bz2_panic_jmp;
	extern volatile int bz2_panic_errcode;
}

#include <cstdlib>

#ifdef ESP_PLATFORM
	#include <esp_heap_caps.h>
#endif

bool RNS::bz2_decompress(const Bytes& compressed, size_t expected_size, Bytes& out, size_t max_size) {
	if (expected_size == 0) {
		out = Bytes();
		return true;
	}
	if (expected_size > max_size) {
		ERRORF("bz2_decompress: refusing to decompress %u bytes (limit %u)", (unsigned)expected_size, (unsigned)max_size);
		return false;
	}
	if (compressed.size() == 0) {
		ERROR("bz2_decompress: empty compressed input");
		return false;
	}

#ifdef ESP_PLATFORM
	// Fail-fast pre-check (2026-09-30). A bzip2 stream's first 4 bytes are
	// always "BZh" followed by an ASCII '1'-'9' declaring blockSize100k -
	// the compressor's chosen block size, which is what bzip2's internal
	// decompression state size actually depends on, NOT the size of the
	// original (or compressed) data. A tiny message compressed at a
	// sender's default/max level can demand just as much decompression
	// memory as a huge one - measured on real hardware at ~1.8MB for a
	// level-9 (900KB block) stream under the small=1 decoder below, i.e.
	// roughly 200KB per blockSize100k unit. There's no way to ask the
	// sender for a smaller block size - Reticulum's Resource protocol has
	// no such negotiation - so on a PSRAM-constrained board the best
	// available response is failing fast and clearly here, rather than
	// spending time inside BZ2_bzBuffToBuffDecompress only to hit its own
	// internal BZ_MEM_ERROR (which this check would otherwise be
	// indistinguishable from in the logs).
	if (compressed.size() >= 4 && compressed.data()[0] == 'B' && compressed.data()[1] == 'Z' && compressed.data()[2] == 'h') {
		int block_size_100k = compressed.data()[3] - '0';
		if (block_size_100k >= 1 && block_size_100k <= 9) {
			size_t estimated_need = (size_t)block_size_100k * 200000;
			size_t largest_free = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
			if (estimated_need > largest_free) {
				ERRORF("bz2_decompress: refusing - stream's block size (%d00KB) needs an estimated %u bytes to decompress, largest free PSRAM block is only %u bytes",
					block_size_100k, (unsigned)estimated_need, (unsigned)largest_free);
				return false;
			}
		}
	}
#endif

	// Plain malloc, not a Bytes/PSRAM-allocator buffer - this is a
	// short-lived scratch destination for BZ2_bzBuffToBuffDecompress to
	// write into directly; it's copied into `out` (a real Bytes, via
	// the constructor below) and freed before returning either way.
	uint8_t* dest = (uint8_t*)malloc(expected_size);
	if (dest == nullptr) {
		ERRORF("bz2_decompress: failed to allocate %u byte output buffer", (unsigned)expected_size);
		return false;
	}

	// setjmp must not have any C++ object with a non-trivial destructor
	// in scope across it in this function - dest above is a raw
	// pointer, and nothing else is constructed between here and the
	// matching free() calls below, so unwinding via longjmp is safe.
	if (setjmp(bz2_panic_jmp) != 0) {
		ERRORF("bz2_decompress: internal error %d (corrupt or malicious stream)", bz2_panic_errcode);
		free(dest);
		return false;
	}

	unsigned int destLen = (unsigned int)expected_size;
	int ret = BZ2_bzBuffToBuffDecompress(
		(char*)dest, &destLen,
		(char*)const_cast<uint8_t*>(compressed.data()), (unsigned int)compressed.size(),
		// small=1 (2026-09-30): the fast (small=0) decoder needs ~400KB of
		// internal working memory - fine on boards with generous PSRAM
		// (e.g. MeshAdventurer-S3's 8MB) but BZ_MEM_ERROR (ret=-3 below)
		// confirmed on hardware on a board with only 2MB PSRAM already
		// shared with BLE/GNSS/display/RNS's own PSRAM containers -
		// identical received bytes on both boards (verified byte-for-byte
		// via live capture), only the memory-constrained board failed, and
		// -3 is bzip2's own "couldn't allocate working memory" code, not a
		// data error. small=1 uses ~2.3 bytes/block-byte instead of ~3.7 -
		// slower decompression, fine here since this only runs when
		// receiving a large message, not a hot path.
		1 /* small - low-memory decoder, see project_urns_bz2_small_mode memory */,
		0 /* verbosity - VPrintf* are no-ops in this port regardless */
	);

	if (ret != BZ_OK) {
		ERRORF("bz2_decompress: BZ2_bzBuffToBuffDecompress failed, ret=%d", ret);
		free(dest);
		return false;
	}

	if (destLen != expected_size) {
		WARNINGF("bz2_decompress: decompressed %u bytes, advertised size was %u", destLen, (unsigned)expected_size);
	}

	out = Bytes(dest, destLen);
	free(dest);
	return true;
}
