// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ethernet_backend_mock.h"

#include <spdlog/spdlog.h>

EthernetBackendMock::EthernetBackendMock() {
    spdlog::debug("[EthernetMock] Mock backend created");
}

EthernetBackendMock::~EthernetBackendMock() {
    // Use fprintf - spdlog may be destroyed during static cleanup
    fprintf(stderr, "[EthernetMock] Mock backend destroyed\n");
}

bool EthernetBackendMock::has_interface() {
    // Always report Ethernet available in mock mode
    return true;
}

EthernetInfo EthernetBackendMock::get_info() {
    // Return static fake data
    EthernetInfo info;
    info.connected = true;
    info.interface = "en0";
    info.ip_address = "192.168.1.150";
    info.mac_address = "aa:bb:cc:dd:ee:ff";
    info.status = "Connected";

    spdlog::trace("[EthernetMock] get_info() → {} ({})", info.ip_address, info.status);
    return info;
}
