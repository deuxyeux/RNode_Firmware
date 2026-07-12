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

char eth_hostname[11];

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
    eth_spi.begin(pin_eth_sclk, pin_eth_miso, pin_eth_mosi, pin_eth_cs);
    WiFi.onEvent(onEthEvent);
    ETH.begin(
        ETH_PHY_W5500,
        0,
        pin_eth_cs,
        pin_eth_int,
        pin_eth_rst,
        eth_spi,
        20000000
    );
    remote_listener.begin();
    remote_listener.setTimeout(WR_SOCKET_TIMEOUT);
    wr_state = WR_STATE_ON;
}

bool ethernet_is_connected() { return eth_is_connected; }
