#pragma once

// LXMF FIELD_AUDIO (key 0x07): value is msgpack [mode, bin] where mode is an
// LXMF.AM_* byte and bin is the codec payload (for Codec2 modes, raw frames
// concatenated with no header - see Sideband's audioproc.py). Fields are kept
// as raw msgpack by LXMessage, so this decodes the one shape we care about
// without pulling a msgpack dependency into callers.

#include "LXMessage.h"

namespace LXMF {

constexpr uint8_t FIELD_AUDIO = 0x07;

struct AudioField {
	uint8_t mode = 0;
	const uint8_t* data = nullptr;  // points into the message's own field storage
	size_t size = 0;
};

// Returns true and fills `out` if the message carries a well-formed
// [mode, bin] audio field. `out.data` is only valid while `msg` is alive.
inline bool parse_audio_field(const LXMessage& msg, AudioField& out) {
	const uint8_t key_byte = FIELD_AUDIO;
	const RNS::Bytes key(&key_byte, 1);
	const RNS::Bytes* v = msg.fields_get(key);
	if (!v) return false;
	const uint8_t* p = v->data();
	size_t n = v->size();
	if (n < 4 || p[0] != 0x92) return false;  // fixarray(2)
	size_t i = 1;
	// mode: positive fixint or uint8
	if (p[i] < 0x80) { out.mode = p[i]; i += 1; }
	else if (p[i] == 0xCC && i + 1 < n) { out.mode = p[i + 1]; i += 2; }
	else return false;
	if (i >= n) return false;
	size_t len = 0;
	uint8_t t = p[i++];
	if (t == 0xC4) { if (i + 1 > n) return false; len = p[i]; i += 1; }
	else if (t == 0xC5) { if (i + 2 > n) return false; len = ((size_t)p[i] << 8) | p[i + 1]; i += 2; }
	else if (t == 0xC6) { if (i + 4 > n) return false; len = ((size_t)p[i] << 24) | ((size_t)p[i + 1] << 16) | ((size_t)p[i + 2] << 8) | p[i + 3]; i += 4; }
	else return false;
	if (len > n - i) return false;
	out.data = p + i;
	out.size = len;
	return true;
}

// Encode [mode, bin] as the raw msgpack value fields_set() expects for
// FIELD_AUDIO (what umsgpack produces for [int, bytes] in Python LXMF).
inline RNS::Bytes build_audio_field(uint8_t mode, const uint8_t* data, size_t len) {
	RNS::Bytes out;
	out.append((uint8_t)0x92);  // fixarray(2)
	if (mode < 0x80) {
		out.append(mode);
	} else {
		out.append((uint8_t)0xCC);
		out.append(mode);
	}
	if (len <= 0xFF) {
		out.append((uint8_t)0xC4);
		out.append((uint8_t)len);
	} else if (len <= 0xFFFF) {
		out.append((uint8_t)0xC5);
		out.append((uint8_t)(len >> 8));
		out.append((uint8_t)len);
	} else {
		out.append((uint8_t)0xC6);
		out.append((uint8_t)(len >> 24));
		out.append((uint8_t)(len >> 16));
		out.append((uint8_t)(len >> 8));
		out.append((uint8_t)len);
	}
	out.append(data, len);
	return out;
}

}  // namespace LXMF
