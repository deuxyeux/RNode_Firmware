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

#include "../Bytes.h"

namespace RNS { namespace Cryptography {

	// PBKDF2-HMAC-SHA256 (RFC 8018). Not part of the upstream attermann/Crypto
	// library, so built here directly on top of the existing HMAC wrapper.
	const Bytes pbkdf2_hmac_sha256(const Bytes& password, const Bytes& salt, uint32_t iterations, size_t dklen);

} }
