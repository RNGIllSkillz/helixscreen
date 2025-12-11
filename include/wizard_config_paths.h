// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file wizard_config_paths.h
 * @brief Centralized configuration paths for wizard screens
 *
 * Defines all JSON configuration paths used by wizard screens to eliminate
 * hardcoded string literals and reduce typo risk.
 *
 * Paths now use structured format under /printers/default_printer/... to align
 * with the expected structure checked by Config::is_wizard_required().
 */

namespace helix {
namespace wizard {
// Printer identification
constexpr const char* DEFAULT_PRINTER = "/default_printer";
constexpr const char* PRINTER_NAME = "/printers/default_printer/name";
constexpr const char* PRINTER_TYPE = "/printers/default_printer/type";

// Bed hardware
constexpr const char* BED_HEATER = "/printers/default_printer/heater/bed";
constexpr const char* BED_SENSOR = "/printers/default_printer/sensor/bed";

// Hotend hardware
constexpr const char* HOTEND_HEATER = "/printers/default_printer/heater/hotend";
constexpr const char* HOTEND_SENSOR = "/printers/default_printer/sensor/hotend";

// Fan hardware
constexpr const char* HOTEND_FAN = "/printers/default_printer/fan/hotend";
constexpr const char* PART_FAN = "/printers/default_printer/fan/part";

// LED hardware
constexpr const char* LED_STRIP = "/printers/default_printer/led/strip";

// Network configuration
// Note: Connection screen constructs full path dynamically using default_printer
// e.g., "/printers/" + default_printer + "/moonraker_host"
// These constants are for direct access in ui_wizard_complete()
constexpr const char* MOONRAKER_HOST = "/printers/default_printer/moonraker_host";
constexpr const char* MOONRAKER_PORT = "/printers/default_printer/moonraker_port";
constexpr const char* WIFI_SSID = "/wifi/ssid";
constexpr const char* WIFI_PASSWORD = "/wifi/password";
} // namespace wizard
} // namespace helix
