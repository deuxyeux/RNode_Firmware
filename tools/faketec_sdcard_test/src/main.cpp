// Isolated hardware bring-up test for an SPI SD card wired to spare pads on
// a ProMicro/FakeTec (nice!nano v2) board. Not part of RNode_Firmware's own
// build - see ../README.md.
//
// Wiring (see README.md for the naming - these are the nice!nano's own
// D3/D4/D5/D0 pads, which happen to carry the "GPS_TX/GPS_RX/GPS_EN" labels
// in this board's Meshtastic pinout since nothing else claims them here):
//
//   SD CLK  -> D5 (P0.24, aka "GPS_EN")
//   SD MISO -> D4 (P0.22, aka "GPS_TX")
//   SD MOSI -> D3 (P0.20, aka "GPS_RX")
//   SD CS   -> D0 (P0.06)
//
// NOTE: GPS_TX/GPS_RX are swapped here vs the generic Meshtastic reference
// table (nrf52_promicro_diy_tcxo/variant.h, which has GPS_TX=D3/P0.20,
// GPS_RX=D4/P0.22) - confirmed against this board's own physical silkscreen,
// which marks the GPS_RX pad as D3. Hardware bring-up (real board, 'h' hold-
// test) caught the mismatch: SCK/CS toggled fine, but the wire on the
// GPS_RX/MOSI pad sat on what the firmware was treating as MISO (an input
// it never drives), making it read as floating.
//
// None of these are the nice!nano core's native hardware-SPI pins (that's
// D2/D3/D4/D5 = SCK/MISO/MOSI/SS) - the nRF52840's SPIM peripheral can mux
// to arbitrary GPIOs though, so SPIClass::setPins() remaps the existing
// `SPI` object's SCK/MISO/MOSI before begin() instead of bit-banging.

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>

#define PIN_SD_SCK  5  // D5 / P0.24 ("GPS_EN")
#define PIN_SD_MISO 4  // D4 / P0.22 ("GPS_TX")
#define PIN_SD_MOSI 3  // D3 / P0.20 ("GPS_RX")
#define PIN_SD_CS   0  // D0 / P0.06

#define TEST_FILE_PATH "/faketec_sdtest.bin"
#define TEST_FILE_SIZE 4096

SdFat sd;
bool sd_ready = false;

void print_help() {
	Serial.println();
	Serial.println(F("Faketec SD card SPI bring-up test"));
	Serial.println(F("  i - init/re-init SD card, print card + volume info"));
	Serial.println(F("  m - format the card (FAT/exFAT, auto-selected by size) - DESTRUCTIVE"));
	Serial.println(F("  w - write test file (4KB pseudo-random pattern)"));
	Serial.println(F("  r - read test file back and verify against pattern"));
	Serial.println(F("  l - list root directory"));
	Serial.println(F("  x - delete test file"));
	Serial.println(F("  f - full auto test: init -> write -> read/verify -> list -> cleanup"));
	Serial.println(F("  p - raw GPIO probe (bit-banged CMD0, bypasses SPI peripheral/SdFat)"));
	Serial.println(F("  h - hold each output pin LOW/HIGH for 4s each, for multimeter probing"));
	Serial.println(F("  ? - print this help"));
	Serial.println();
}

// Bit-banged SPI mode 0, manual digitalWrite/digitalRead on the raw pins -
// no nrfx_spim peripheral, no SdFat. Isolates "is the wiring/card even
// there" from "is the SPIM pin mux / SdFat driver plumbing correct".
uint8_t bitbang_xfer(uint8_t out) {
	uint8_t in = 0;
	for (int8_t bit = 7; bit >= 0; bit--) {
		digitalWrite(PIN_SD_MOSI, (out >> bit) & 1);
		delayMicroseconds(5);
		digitalWrite(PIN_SD_SCK, HIGH);
		delayMicroseconds(5);
		in = (in << 1) | (digitalRead(PIN_SD_MISO) & 1);
		digitalWrite(PIN_SD_SCK, LOW);
		delayMicroseconds(5);
	}
	return in;
}

void raw_gpio_probe() {
	Serial.println(F("[probe] raw bit-banged CMD0 (SPI peripheral/SdFat bypassed)"));

	pinMode(PIN_SD_SCK, OUTPUT);
	pinMode(PIN_SD_MOSI, OUTPUT);
	pinMode(PIN_SD_CS, OUTPUT);

	digitalWrite(PIN_SD_SCK, LOW);
	digitalWrite(PIN_SD_MOSI, HIGH);
	digitalWrite(PIN_SD_CS, HIGH);
	delay(10);

	// Float MISO under each internal bias in turn to tell "genuinely
	// floating/disconnected pin" (follows whichever bias is active) apart
	// from "something external is actually driving it" (same level either
	// way, overpowering the ~13k internal pull).
	pinMode(PIN_SD_MISO, INPUT_PULLUP);
	delay(2);
	bool miso_pullup = digitalRead(PIN_SD_MISO);
	pinMode(PIN_SD_MISO, INPUT_PULLDOWN);
	delay(2);
	bool miso_pulldown = digitalRead(PIN_SD_MISO);
	pinMode(PIN_SD_MISO, INPUT_PULLUP);

	Serial.print(F("[probe] MISO level w/ internal pullup: "));
	Serial.print(miso_pullup ? F("HIGH") : F("LOW"));
	Serial.print(F("  w/ internal pulldown: "));
	Serial.println(miso_pulldown ? F("HIGH") : F("LOW"));
	if (miso_pullup != miso_pulldown) {
		Serial.println(F("[probe] MISO follows the internal bias both ways -> nothing is"));
		Serial.println(F("[probe] overpowering the ~13k internal pull right now. NOTE: this"));
		Serial.println(F("[probe] is CS-deselected (Hi-Z) - a card correctly tri-states DO"));
		Serial.println(F("[probe] here unless the module itself has a MISO pull-up resistor,"));
		Serial.println(F("[probe] so this alone does NOT prove a bad connection."));
	} else {
		Serial.println(F("[probe] MISO holds the same level under both biases ->"));
		Serial.println(F("[probe] something external IS driving this pin."));
	}

	// >=74 clock cycles with CS high and MOSI high, per SD SPI power-up spec.
	for (int i = 0; i < 10; i++) bitbang_xfer(0xFF);

	// CMD0 (GO_IDLE_STATE), valid CRC 0x95, sent with CS low.
	digitalWrite(PIN_SD_CS, LOW);
	delayMicroseconds(5);
	uint8_t cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
	for (uint8_t b : cmd0) bitbang_xfer(b);

	Serial.print(F("[probe] CMD0 response bytes: "));
	bool got_reply = false;
	for (int i = 0; i < 10; i++) {
		uint8_t r = bitbang_xfer(0xFF);
		if (r != 0xFF) got_reply = true;
		Serial.print(F("0x"));
		if (r < 0x10) Serial.print('0');
		Serial.print(r, HEX);
		Serial.print(' ');
	}
	Serial.println();

	digitalWrite(PIN_SD_CS, HIGH);
	bitbang_xfer(0xFF);

	if (!got_reply) {
		Serial.println(F("[probe] all 0xFF - card never responded. Check: card"));
		Serial.println(F("[probe] inserted/seated, 3.3V + GND wired to the card,"));
		Serial.println(F("[probe] and each signal landing on the pin it's supposed to"));
		Serial.println(F("[probe] (see README.md wiring table)."));
	} else if (got_reply) {
		Serial.println(F("[probe] got a non-0xFF byte - card is responding on the bus."));
		Serial.println(F("[probe] (0x01 = idle-state R1, the expected CMD0 reply)"));
	}
}

// Drives each output pin to a known level for several seconds so it can be
// confirmed with a multimeter directly at the pad the wire lands on -
// validates "firmware pin number -> physical pad" independent of the SD
// protocol or card state entirely.
void hold_pin_levels() {
	pinMode(PIN_SD_SCK, OUTPUT);
	pinMode(PIN_SD_MOSI, OUTPUT);
	pinMode(PIN_SD_CS, OUTPUT);

	struct { const char* name; uint8_t pin; } pins[] = {
		{"SCK (D5/P0.24, GPS_EN)", PIN_SD_SCK},
		{"MOSI (D3/P0.20, GPS_RX)", PIN_SD_MOSI},
		{"CS (D0/P0.06)", PIN_SD_CS},
	};

	for (auto& p : pins) {
		digitalWrite(p.pin, LOW);
		Serial.print(F("[hold] "));
		Serial.print(p.name);
		Serial.println(F(" -> LOW for 4s, probe now"));
		delay(4000);

		digitalWrite(p.pin, HIGH);
		Serial.print(F("[hold] "));
		Serial.print(p.name);
		Serial.println(F(" -> HIGH for 4s, probe now"));
		delay(4000);

		digitalWrite(p.pin, LOW);
	}
	Serial.println(F("[hold] done"));
}

// Raw card-level info - safe to call as soon as cardBegin() succeeds, even
// if no filesystem has been (or can be) mounted on top of it yet.
void print_raw_card_info() {
	uint32_t sectors = sd.card()->sectorCount();
	if (sectors == 0) {
		Serial.println(F("[sd] sectorCount() returned 0 - card not readable"));
		return;
	}
	uint64_t size_mb = ((uint64_t)sectors * 512ULL) / (1024ULL * 1024ULL);
	Serial.print(F("[sd] card type: "));
	switch (sd.card()->type()) {
		case SD_CARD_TYPE_SD1:  Serial.println(F("SD1"));  break;
		case SD_CARD_TYPE_SD2:  Serial.println(F("SD2"));  break;
		case SD_CARD_TYPE_SDHC: Serial.println(F("SDHC/SDXC")); break;
		default:                Serial.println(F("unknown")); break;
	}
	Serial.print(F("[sd] sectors: "));
	Serial.print(sectors);
	Serial.print(F("  size: "));
	Serial.print((uint32_t)size_mb);
	Serial.println(F(" MB"));
}

// Filesystem-level info - only valid once volumeBegin() has succeeded.
void print_volume_info() {
	Serial.print(F("[sd] volume FAT type: FAT"));
	Serial.println(sd.vol()->fatType());

	uint32_t free_kb = (uint32_t)(sd.vol()->freeClusterCount() * (uint64_t)sd.vol()->bytesPerCluster() / 1024ULL);
	Serial.print(F("[sd] free space: "));
	Serial.print(free_kb);
	Serial.println(F(" KB"));
}

bool sd_init() {
	Serial.println(F("[sd] initializing SPI + SD card..."));

	pinMode(PIN_SD_CS, OUTPUT);
	digitalWrite(PIN_SD_CS, HIGH);

	// Remap the shared SPI bus's SCK/MISO/MOSI to this board's wiring before
	// SdFat's own begin() call below triggers SPI.begin(). Must happen before
	// that first begin(), since setPins() just rewrites private pin fields -
	// it doesn't re-init already-configured hardware.
	SPI.setPins(PIN_SD_MISO, PIN_SD_SCK, PIN_SD_MOSI);

	SdSpiConfig config(PIN_SD_CS, DEDICATED_SPI, SD_SCK_MHZ(4), &SPI);
	bool card_ok = sd.cardBegin(config);
	Serial.print(F("[sd] cardBegin() (raw SPI/card comms, no filesystem): "));
	Serial.println(card_ok ? F("OK") : F("FAILED"));
	if (card_ok) print_raw_card_info();

	if (!card_ok || !sd.volumeBegin()) {
		Serial.println(F("[sd] sd.begin() FAILED - check wiring/card"));
		sd.initErrorPrint(&Serial);
		sd_ready = false;
		return false;
	}

	Serial.println(F("[sd] sd.begin() OK (card + filesystem both mounted)"));
	print_volume_info();
	sd_ready = true;
	return true;
}

// Formats over cardBegin() alone (no mounted volume required) - used to
// recover a card with no filesystem SdFat recognizes, or wipe a card with
// leftover data from something else entirely (Pi OS, camera, etc).
bool sd_format() {
	if (!sd.card()) {
		Serial.println(F("[sd] no card - run 'i' first (cardBegin() must succeed)"));
		return false;
	}
	Serial.println(F("[sd] formatting - this can take a while on large cards..."));
	uint32_t started = millis();
	bool ok = sd.format(&Serial);
	Serial.print(F("[sd] format() "));
	Serial.print(ok ? F("OK") : F("FAILED"));
	Serial.print(F(" in "));
	Serial.print(millis() - started);
	Serial.println(F(" ms"));
	if (ok) {
		Serial.println(F("[sd] re-run 'i' to mount the fresh filesystem"));
		sd_ready = false;
	}
	return ok;
}

bool sd_write_test_file() {
	if (!sd_ready) {
		Serial.println(F("[sd] not initialized - run 'i' first"));
		return false;
	}

	FsFile file = sd.open(TEST_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC);
	if (!file) {
		Serial.println(F("[sd] open() for write FAILED"));
		return false;
	}

	Serial.print(F("[sd] writing "));
	Serial.print(TEST_FILE_SIZE);
	Serial.println(F(" bytes..."));

	uint32_t started = millis();
	uint8_t buf[256];
	// LFSR-style pseudo-random pattern, seeded so read-back can regenerate
	// and compare it without having to keep the write buffer around.
	uint8_t seed = 0x2A;
	size_t written = 0;
	while (written < TEST_FILE_SIZE) {
		for (size_t i = 0; i < sizeof(buf); i++) {
			seed = (seed << 1) ^ ((seed & 0x80) ? 0x1D : 0x00) ^ (uint8_t)(written + i);
			buf[i] = seed;
		}
		size_t n = sizeof(buf);
		if (written + n > TEST_FILE_SIZE) n = TEST_FILE_SIZE - written;
		size_t w = file.write(buf, n);
		if (w != n) {
			Serial.print(F("[sd] short write at offset "));
			Serial.println(written);
			file.close();
			return false;
		}
		written += w;
	}
	file.flush();
	file.close();

	uint32_t elapsed = millis() - started;
	Serial.print(F("[sd] wrote "));
	Serial.print(written);
	Serial.print(F(" bytes in "));
	Serial.print(elapsed);
	Serial.println(F(" ms"));
	return true;
}

bool sd_read_verify_test_file() {
	if (!sd_ready) {
		Serial.println(F("[sd] not initialized - run 'i' first"));
		return false;
	}

	FsFile file = sd.open(TEST_FILE_PATH, O_RDONLY);
	if (!file) {
		Serial.println(F("[sd] open() for read FAILED (does the file exist? run 'w' first)"));
		return false;
	}

	size_t size = file.size();
	Serial.print(F("[sd] reading back "));
	Serial.print(size);
	Serial.println(F(" bytes..."));

	if (size != TEST_FILE_SIZE) {
		Serial.print(F("[sd] MISMATCH: expected size "));
		Serial.print(TEST_FILE_SIZE);
		Serial.print(F(", got "));
		Serial.println(size);
		file.close();
		return false;
	}

	uint32_t started = millis();
	uint8_t buf[256];
	uint8_t seed = 0x2A;
	size_t checked = 0;
	bool ok = true;
	while (checked < TEST_FILE_SIZE) {
		size_t n = sizeof(buf);
		if (checked + n > TEST_FILE_SIZE) n = TEST_FILE_SIZE - checked;
		int r = file.read(buf, n);
		if (r != (int)n) {
			Serial.print(F("[sd] short read at offset "));
			Serial.println(checked);
			ok = false;
			break;
		}
		for (size_t i = 0; i < n; i++) {
			seed = (seed << 1) ^ ((seed & 0x80) ? 0x1D : 0x00) ^ (uint8_t)(checked + i);
			if (buf[i] != seed) {
				Serial.print(F("[sd] MISMATCH at offset "));
				Serial.print(checked + i);
				Serial.print(F(": expected 0x"));
				Serial.print(seed, HEX);
				Serial.print(F(", got 0x"));
				Serial.println(buf[i], HEX);
				ok = false;
				break;
			}
		}
		if (!ok) break;
		checked += n;
	}
	file.close();

	uint32_t elapsed = millis() - started;
	if (ok) {
		Serial.print(F("[sd] verified "));
		Serial.print(checked);
		Serial.print(F(" bytes OK in "));
		Serial.print(elapsed);
		Serial.println(F(" ms"));
	} else {
		Serial.println(F("[sd] VERIFY FAILED"));
	}
	return ok;
}

void sd_list_dir() {
	if (!sd_ready) {
		Serial.println(F("[sd] not initialized - run 'i' first"));
		return;
	}
	Serial.println(F("[sd] root directory:"));
	sd.ls(&Serial, LS_SIZE | LS_DATE);
}

bool sd_delete_test_file() {
	if (!sd_ready) {
		Serial.println(F("[sd] not initialized - run 'i' first"));
		return false;
	}
	if (!sd.exists(TEST_FILE_PATH)) {
		Serial.println(F("[sd] test file does not exist, nothing to delete"));
		return true;
	}
	if (!sd.remove(TEST_FILE_PATH)) {
		Serial.println(F("[sd] remove() FAILED"));
		return false;
	}
	Serial.println(F("[sd] test file deleted"));
	return true;
}

void run_full_test() {
	Serial.println(F("=== FULL TEST START ==="));
	bool ok = sd_init();
	if (ok) ok = sd_write_test_file();
	if (ok) ok = sd_read_verify_test_file();
	if (ok) sd_list_dir();
	if (ok) ok = sd_delete_test_file();
	Serial.println(ok ? F("=== FULL TEST: PASS ===") : F("=== FULL TEST: FAIL ==="));
}

void setup() {
	Serial.begin(115200);
	uint32_t wait_started = millis();
	while (!Serial && millis() - wait_started < 5000) delay(10);
	delay(200);
	print_help();
}

void loop() {
	if (Serial.available()) {
		char c = Serial.read();
		switch (c) {
			case 'i': sd_init(); break;
			case 'm': sd_format(); break;
			case 'w': sd_write_test_file(); break;
			case 'r': sd_read_verify_test_file(); break;
			case 'l': sd_list_dir(); break;
			case 'x': sd_delete_test_file(); break;
			case 'f': run_full_test(); break;
			case 'p': raw_gpio_probe(); break;
			case 'h': hold_pin_levels(); break;
			case '?': print_help(); break;
			case '\r': case '\n': break;
			default:
				Serial.print(F("unknown command: "));
				Serial.println(c);
				print_help();
				break;
		}
	}
}
