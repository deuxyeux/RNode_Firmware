/*
 * Copyright (c) 2023 Chad Attermann
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include "CBC.h"

#include "../Bytes.h"

#include <AES.h>
#include <GCM.h>

#include <stdexcept>

#ifdef ESP_PLATFORM
	#include <mbedtls/gcm.h>
#endif

namespace RNS { namespace Cryptography {

	class AES_128_CBC {

	public:
		static inline const Bytes encrypt(const Bytes& plaintext, const Bytes& key, const Bytes& iv) {
			CBC<AES128> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			Bytes ciphertext;
			cbc.encrypt(ciphertext.writable(plaintext.size()), plaintext.data(), plaintext.size());
			return ciphertext;
		}

		static inline const Bytes decrypt(const Bytes& ciphertext, const Bytes& key, const Bytes& iv) {
			CBC<AES128> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			Bytes plaintext;
			cbc.decrypt(plaintext.writable(ciphertext.size()), ciphertext.data(), ciphertext.size());
			return plaintext;
		}

		// EXPERIMENTAL - overwrites passed buffer
		static inline void inplace_encrypt(Bytes& plaintext, const Bytes& key, const Bytes& iv) {
			CBC<AES128> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			cbc.encrypt((uint8_t*)plaintext.data(), plaintext.data(), plaintext.size());
		}

		// EXPERIMENTAL - overwrites passed buffer
		static inline void inplace_decrypt(Bytes& ciphertext, const Bytes& key, const Bytes& iv) {
			CBC<AES128> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			cbc.decrypt((uint8_t*)ciphertext.data(), ciphertext.data(), ciphertext.size());
		}

	};

	class AES_256_CBC {

	public:
		static inline const Bytes encrypt(const Bytes& plaintext, const Bytes& key, const Bytes& iv) {
			CBC<AES256> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			Bytes ciphertext;
			cbc.encrypt(ciphertext.writable(plaintext.size()), plaintext.data(), plaintext.size());
			return ciphertext;
		}

		static inline const Bytes decrypt(const Bytes& ciphertext, const Bytes& key, const Bytes& iv) {
			CBC<AES256> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			Bytes plaintext;
			cbc.decrypt(plaintext.writable(ciphertext.size()), ciphertext.data(), ciphertext.size());
			return plaintext;
		}

		// EXPERIMENTAL - overwrites passed buffer
		static inline void inplace_encrypt(Bytes& plaintext, const Bytes& key, const Bytes& iv) {
			CBC<AES256> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			cbc.encrypt((uint8_t*)plaintext.data(), plaintext.data(), plaintext.size());
		}

		// EXPERIMENTAL - overwrites passed buffer
		static inline void inplace_decrypt(Bytes& ciphertext, const Bytes& key, const Bytes& iv) {
			CBC<AES256> cbc;
			cbc.setKey(key.data(), key.size());
			cbc.setIV(iv.data(), iv.size());
			cbc.decrypt((uint8_t*)ciphertext.data(), ciphertext.data(), ciphertext.size());
		}

	};

	// AES-256-GCM AEAD. ciphertext output/input from encrypt()/decrypt() is
	// "ciphertext || tag" (tag appended, GCM_TAG_SIZE bytes, 128-bit tag).
	// decrypt() throws std::runtime_error if the tag fails to verify - callers
	// must not use the returned plaintext unless decrypt() returns normally.
	//
	// On ESP32, delegates to mbedtls (mbedtls_gcm_crypt_and_tag/auth_decrypt)
	// instead of the Crypto library's GCM<AES256> - this build has
	// CONFIG_MBEDTLS_HARDWARE_AES=1, so mbedtls's AES runs on the ESP32-S3's
	// AES hardware peripheral. Confirmed on-device during phase (a)
	// validation: correctness matches (same KAT, same tamper-rejection
	// behavior) and it's substantially faster than the software path -
	// ~2.7x at 64B (identity-sized) and ~4.8x at 256B (message-sized),
	// scaling better with size since the hardware engine's per-call setup
	// cost amortizes over more data (unlike PBKDF2's tiny 32-64 byte HMAC
	// rounds, where hardware SHA's per-call overhead ate most of the
	// theoretical win - AES-GCM here processes the full buffer in one
	// hardware-accelerated pass, not many tiny per-round calls).
	class AES_256_GCM {

	public:
		static const size_t KEY_SIZE = 32;
		static const size_t TAG_SIZE = 16;

		static inline const Bytes encrypt(const Bytes& plaintext, const Bytes& key, const Bytes& nonce, const Bytes& aad = {Bytes::NONE}) {
#ifdef ESP_PLATFORM
			mbedtls_gcm_context ctx;
			mbedtls_gcm_init(&ctx);
			int ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key.data(), (unsigned int)(key.size() * 8));
			if (ret != 0) {
				mbedtls_gcm_free(&ctx);
				throw std::runtime_error("mbedtls_gcm_setkey failed");
			}
			Bytes ciphertext;
			uint8_t* out = ciphertext.writable(plaintext.size());
			uint8_t tag[TAG_SIZE];
			ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, plaintext.size(),
				nonce.data(), nonce.size(), aad.data(), aad.size(),
				plaintext.data(), out, TAG_SIZE, tag);
			mbedtls_gcm_free(&ctx);
			if (ret != 0) {
				throw std::runtime_error("mbedtls_gcm_crypt_and_tag failed");
			}
			ciphertext.append(tag, TAG_SIZE);
			return ciphertext;
#else
			GCM<AES256> gcm;
			gcm.setKey(key.data(), key.size());
			gcm.setIV(nonce.data(), nonce.size());
			if (aad) {
				gcm.addAuthData(aad.data(), aad.size());
			}
			Bytes ciphertext;
			gcm.encrypt(ciphertext.writable(plaintext.size()), plaintext.data(), plaintext.size());
			uint8_t tag[TAG_SIZE];
			gcm.computeTag(tag, TAG_SIZE);
			ciphertext.append(tag, TAG_SIZE);
			return ciphertext;
#endif
		}

		// ciphertext must be "ciphertext || tag" as produced by encrypt() above.
		static inline const Bytes decrypt(const Bytes& ciphertext, const Bytes& key, const Bytes& nonce, const Bytes& aad = {Bytes::NONE}) {
			if (ciphertext.size() < TAG_SIZE) {
				throw std::invalid_argument("GCM ciphertext shorter than tag size");
			}
			size_t body_size = ciphertext.size() - TAG_SIZE;
#ifdef ESP_PLATFORM
			mbedtls_gcm_context ctx;
			mbedtls_gcm_init(&ctx);
			int ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key.data(), (unsigned int)(key.size() * 8));
			if (ret != 0) {
				mbedtls_gcm_free(&ctx);
				throw std::runtime_error("mbedtls_gcm_setkey failed");
			}
			Bytes plaintext;
			uint8_t* out = plaintext.writable(body_size);
			ret = mbedtls_gcm_auth_decrypt(&ctx, body_size,
				nonce.data(), nonce.size(), aad.data(), aad.size(),
				ciphertext.data() + body_size, TAG_SIZE,
				ciphertext.data(), out);
			mbedtls_gcm_free(&ctx);
			if (ret != 0) {
				throw std::runtime_error("GCM authentication tag verification failed");
			}
			return plaintext;
#else
			GCM<AES256> gcm;
			gcm.setKey(key.data(), key.size());
			gcm.setIV(nonce.data(), nonce.size());
			if (aad) {
				gcm.addAuthData(aad.data(), aad.size());
			}
			Bytes plaintext;
			gcm.decrypt(plaintext.writable(body_size), ciphertext.data(), body_size);
			if (!gcm.checkTag(ciphertext.data() + body_size, TAG_SIZE)) {
				throw std::runtime_error("GCM authentication tag verification failed");
			}
			return plaintext;
#endif
		}

	};

} }
