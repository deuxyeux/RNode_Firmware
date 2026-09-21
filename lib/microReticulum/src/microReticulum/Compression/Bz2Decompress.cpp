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
		0 /* small - use the FAST (tt[]) decoder, PSRAM covers the memory */,
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
