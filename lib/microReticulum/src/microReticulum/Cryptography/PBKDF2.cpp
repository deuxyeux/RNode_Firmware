#include "PBKDF2.h"

#include <Arduino.h>
#include <SHA256.h>
#include <stdexcept>

#ifdef ESP_PLATFORM
	#include <freertos/FreeRTOS.h>
	#include <freertos/task.h>
	#include <mbedtls/pkcs5.h>
	#include <mbedtls/md.h>
#endif

using namespace RNS;

// Iteration counts in the hundreds of thousands (needed to make offline
// brute-force costly) take long enough that, run without ever yielding, they
// starve the idle task long enough to trip the ESP32 idle-task watchdog -
// confirmed by an on-device reset during phase (a) validation.
// yield()/taskYIELD() is NOT sufficient here - it only offers the scheduler a
// chance to switch away, but since this loop's own task is immediately
// ready again, the scheduler can (and on ESP32, does) just resume it without
// ever actually running the idle task, so the watchdog kept tripping even
// with periodic yield() calls (confirmed on-device: every ~5s throughout a
// run, despite yielding every 2048 iterations at ~160us/iteration, i.e.
// far more often than every 5s). vTaskDelay(1) forces a real tick-blocked
// suspension, which guarantees the idle task gets to run.
static const uint32_t PBKDF2_YIELD_INTERVAL = 2048;

// RFC 8018 PBKDF2, PRF = HMAC-SHA256.
//
// On ESP32, delegates to mbedtls's mbedtls_pkcs5_pbkdf2_hmac_ext() - this
// build has CONFIG_MBEDTLS_HARDWARE_SHA=1, so mbedtls's SHA256 (and
// therefore this PBKDF2) runs on the ESP32-S3's SHA hardware peripheral
// instead of the attermann/Crypto library's pure-software implementation
// used on other platforms. Confirmed on-device during phase (a) validation:
// a real, consistent ~84us/iteration (linear from 1,000 through 300,000
// iterations, negligible fixed per-call overhead) vs. the software
// fallback's ~161us/iteration - roughly 1.9x, not the order-of-magnitude
// win hardware SHA gives for bulk hashing (HMAC-SHA256 here processes only
// ~32-64 bytes per round, so the hardware engine's per-call setup/lock
// overhead eats into the theoretical speedup).
//
// NOTE: this is a single blocking call with no internal yield point this
// code controls - at ~84us/iteration an iteration count in the hundreds of
// thousands can still run long enough to trip the (non-fatal, auto-
// recovering) ESP32 idle-task watchdog, same as the software path. Unlike
// the software path, there's no way to inject a periodic vTaskDelay(1)
// inside mbedtls's implementation - keep this in mind if a future caller
// picks a very high iteration count.
const Bytes Cryptography::pbkdf2_hmac_sha256(const Bytes& password, const Bytes& salt, uint32_t iterations, size_t dklen) {

	if (iterations == 0) {
		throw std::invalid_argument("PBKDF2 iteration count must be > 0");
	}
	if (dklen == 0) {
		throw std::invalid_argument("PBKDF2 output length must be > 0");
	}

#ifdef ESP_PLATFORM
	Bytes derived;
	uint8_t* out = derived.writable(dklen);
	int ret = mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256,
		password.data(), password.size(), salt.data(), salt.size(),
		iterations, (uint32_t)dklen, out);
	if (ret != 0) {
		throw std::runtime_error("mbedtls_pkcs5_pbkdf2_hmac_ext failed");
	}
	return derived;
#else
	// Portable software fallback (no platform-specific hardware-accelerated
	// SHA256 available) - built directly on the Crypto library's SHA256
	// (not the RNS::Cryptography::HMAC wrapper, and not per-round Bytes
	// allocations) - an earlier version constructed a new HMAC wrapper
	// (and, inside it, a new heap-allocated SHA256 Hash object) on every
	// single inner-loop round. On-device measurement during phase (a)
	// validation showed that costing ~232us/iteration (100,000 iterations
	// took 23.2s), almost entirely the per-round heap alloc/free, not the
	// actual SHA256 compression work. Reusing one stack-allocated SHA256
	// object across the whole inner loop via resetHMAC()/update()/
	// finalizeHMAC(), and keeping U/T as fixed 32-byte stack buffers
	// instead of per-round Bytes, removes that allocation churn - measured
	// afterward at ~161us/iteration, still far slower than hardware SHA.
	const size_t hlen = 32; // SHA-256 digest size
	uint32_t num_blocks = (uint32_t)((dklen + hlen - 1) / hlen);

	Bytes derived;
	derived.reserve(num_blocks * hlen);

	SHA256 hash;

	for (uint32_t block_index = 1; block_index <= num_blocks; block_index++) {

		Bytes block_input(salt);
		uint8_t int_be[4] = {
			(uint8_t)((block_index >> 24) & 0xFF),
			(uint8_t)((block_index >> 16) & 0xFF),
			(uint8_t)((block_index >> 8) & 0xFF),
			(uint8_t)(block_index & 0xFF),
		};
		block_input.append(int_be, 4);

		uint8_t u[hlen];
		uint8_t t[hlen];

		hash.resetHMAC(password.data(), password.size());
		hash.update(block_input.data(), block_input.size());
		hash.finalizeHMAC(password.data(), password.size(), u, hlen);
		memcpy(t, u, hlen);

		for (uint32_t iter = 1; iter < iterations; iter++) {
			hash.resetHMAC(password.data(), password.size());
			hash.update(u, hlen);
			hash.finalizeHMAC(password.data(), password.size(), u, hlen);
			for (size_t i = 0; i < hlen; i++) {
				t[i] ^= u[i];
			}
			if ((iter % PBKDF2_YIELD_INTERVAL) == 0) {
				#ifdef ESP_PLATFORM
					vTaskDelay(1);
				#else
					yield();
				#endif
			}
		}

		derived.append(t, hlen);
	}

	if (derived.size() > dklen) {
		derived = derived.left(dklen);
	}
	return derived;
#endif
}
