// Copyright (C) 2024, Mark Qvist
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <ETH.h>

SPIClass eth_spi(HSPI);
bool eth_is_connected = false;
bool eth_link_up = false;
IPAddress eth_device_ip;

// Set once at boot (see init_ethernet() below) when eth_speed_mode ==
// ETH_SPEED_OFF - lets Display.h's draw_eth_icon() distinguish "off"
// (never even tried to link) from "down" (tried, no link yet) without
// needing to see ETH_SPEED_OFF itself (Display.h is #include'd before
// this file, same reason eth_link_up above is an extern).
bool eth_disabled = false;

// Snapshot of ETH.linkSpeed()/fullDuplex() (esp32 core) taken on link-up -
// plain globals, not queried live from Display.h's draw_eth_icon(), since
// Display.h is #include'd before this file (see eth_link_up above for the
// same reason) and can't see the ETHClass type at all, let alone the ETH
// object, to call those methods itself.
uint16_t eth_link_speed  = 0;
bool     eth_full_duplex = false;

// Persisted forced speed/duplex setting (ETH_SPEED_*, Config.h) - populated
// from EEPROM (ADDR_CONF_ETHSPD) in setup() before init_ethernet() runs, so
// it's already valid by the time init_ethernet() reads it below.
uint8_t eth_speed_mode = ETH_SPEED_AUTO;

char eth_hostname[11];

// Applies whatever's currently in ADDR_CONF_ETH_IP/NM to the interface - a
// static IP/netmask if both are valid, otherwise ETH.config() with its
// all-zero defaults, which (see NetworkInterface::config(), esp32 core)
// stops and restarts the DHCP client rather than actually zeroing the
// address. Safe to call any time after ETH.begin() (unlike the speed/
// duplex setters, config() only needs the netif to already exist, which
// begin() is what creates) - used both at boot (init_ethernet() below) and
// live from the menu whenever IP Address/Netmask is changed (see
// ethaddr_conf_save(), Utilities.h). addr4_read() (Utilities.h) is shared
// with WiFi's own static IP (ADDR_CONF_IP/NM, Remote.h).
void eth_apply_addr_config() {
    uint8_t ip[4]; uint8_t nm[4];
    if (addr4_read(ADDR_CONF_ETH_IP, ip) && addr4_read(ADDR_CONF_ETH_NM, nm)) {
        IPAddress eth_static_ip(ip[0], ip[1], ip[2], ip[3]);
        IPAddress eth_static_nm(nm[0], nm[1], nm[2], nm[3]);
        ETH.config(eth_static_ip, eth_static_ip, eth_static_nm);
    } else {
        ETH.config();
    }
}

void onEthEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            #if HAS_BLUETOOTH == true || HAS_BLE == true
            snprintf(eth_hostname, sizeof(eth_hostname), "rnode-%02x%02x", (uint8_t)bt_dh[14], (uint8_t)bt_dh[15]);
            #else
            strncpy(eth_hostname, "rnode", sizeof(eth_hostname));
            eth_hostname[sizeof(eth_hostname)-1] = 0x00;
            #endif
            ETH.setHostname(eth_hostname);
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            eth_link_up = true;
            // Safe to query here - the W5500 PHY driver (esp_eth_phy_w5500.c)
            // updates its internal speed/duplex state before it ever raises
            // the link-up event that gets us here, same guarantee the RNode
            // Settings menu's Link Status field relies on querying live.
            eth_link_speed  = ETH.linkSpeed();
            eth_full_duplex = ETH.fullDuplex();
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            eth_is_connected = true;
            eth_device_ip = ETH.localIP();
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
        case ARDUINO_EVENT_ETH_STOP:
            eth_is_connected = false;
            eth_link_up = false;
            break;
        default:
            break;
    }
}

void init_ethernet() {
    if (eth_speed_mode == ETH_SPEED_OFF) {
        // The W5500's autonegotiation runs autonomously in hardware as
        // soon as it's powered and out of reset - never calling
        // ETH.begin() wouldn't actually stop it from linking, since that
        // only stops the ESP32 side from ever talking to it over SPI, not
        // the PHY itself (see ETH_SPEED_OFF, Config.h). Holding reset low
        // is the only way to guarantee no link negotiation happens at all.
        // Deliberately skips eth_spi.begin()/remote_listener.begin() too -
        // nothing else here needs the bus or a listener with the
        // interface never coming up.
        pinMode(pin_eth_rst, OUTPUT);
        digitalWrite(pin_eth_rst, LOW);
        eth_disabled = true;
        return;
    }
    eth_spi.begin(pin_eth_sclk, pin_eth_miso, pin_eth_mosi, pin_eth_cs);
    WiFi.onEvent(onEthEvent);
    // Must be set before ETH.begin() - ETHClass::setAutoNegotiation()/
    // setLinkSpeed()/setFullDuplex() (esp32 core) refuse to do anything once
    // the interface has started, so a changed setting only takes effect at
    // boot (see ethspd_conf_save(), Utilities.h, which reboots on change).
    // Leaving autonegotiation on (the default) for AUTO is deliberate - no
    // setter calls needed, matches this board's original always-auto behavior.
    if (eth_speed_mode != ETH_SPEED_AUTO) {
        bool full_duplex = (eth_speed_mode == ETH_SPEED_100_FULL || eth_speed_mode == ETH_SPEED_10_FULL);
        uint16_t speed = (eth_speed_mode == ETH_SPEED_100_FULL || eth_speed_mode == ETH_SPEED_100_HALF) ? 100 : 10;
        ETH.setAutoNegotiation(false);
        ETH.setLinkSpeed(speed);
        ETH.setFullDuplex(full_duplex);
    }
    ETH.begin(
        ETH_PHY_W5500,
        0,
        pin_eth_cs,
        pin_eth_int,
        pin_eth_rst,
        eth_spi,
        20000000
    );
    // Must come after ETH.begin(), the opposite ordering from the speed/
    // duplex setters above - see eth_apply_addr_config() above.
    eth_apply_addr_config();
    remote_listener.begin();
    remote_listener.setTimeout(WR_SOCKET_TIMEOUT);
    wr_state = WR_STATE_ON;
}

bool ethernet_is_connected() { return eth_is_connected; }
