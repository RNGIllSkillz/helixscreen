// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Copyright (C) 2025 356C LLC
 * Author: Preston Brown <pbrown@brown-house.net>
 *
 * This file is part of HelixScreen.
 *
 * HelixScreen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HelixScreen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HelixScreen. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <sstream>
#include <string>

/**
 * @brief Shared printer type definitions for wizard and auto-detection
 *
 * This file defines the canonical list of supported printer types.
 * Used by:
 * - ui_wizard_printer_identify.cpp (roller options, config persistence)
 * - ui_xml/wizard_printer_identify.xml (roller display)
 * - Future: printer_detector.cpp (auto-detection heuristics)
 *
 * Order matters: Index is stored in config as /printer/type_index
 */

namespace PrinterTypes {

/**
 * @brief Printer type names as newline-separated string for LVGL roller
 *
 * Format: Each line is a printer type option (sorted alphabetically by brand)
 * Default selection: Index 26 ("Unknown")
 */
inline constexpr const char* PRINTER_TYPES_ROLLER = "Anycubic Chiron\n"
                                                    "Anycubic i3 Mega\n"
                                                    "Anycubic Kobra\n"
                                                    "Anycubic Vyper\n"
                                                    "Creality CR-10\n"
                                                    "Creality Ender 3\n"
                                                    "Creality Ender 5\n"
                                                    "Creality K1\n"
                                                    "Doron Velta\n"
                                                    "FlashForge Adventurer 5M\n"
                                                    "FlashForge Adventurer 5M Pro\n"
                                                    "FLSUN Delta\n"
                                                    "Prusa i3 MK3\n"
                                                    "Prusa MK4\n"
                                                    "Prusa Mini\n"
                                                    "Qidi X-Max 3\n"
                                                    "Qidi X-Plus 3\n"
                                                    "RatRig V-Core 3\n"
                                                    "RatRig V-Minion\n"
                                                    "Sovol SV06\n"
                                                    "Sovol SV08\n"
                                                    "Voron 0.2\n"
                                                    "Voron 2.4\n"
                                                    "Voron Switchwire\n"
                                                    "Voron Trident\n"
                                                    "Custom/Other\n"
                                                    "Unknown";

/**
 * @brief Number of printer types in the list
 */
inline constexpr int PRINTER_TYPE_COUNT = 27;

/**
 * @brief Default printer type index (Unknown)
 */
inline constexpr int DEFAULT_PRINTER_TYPE_INDEX = 26;

/**
 * @brief Find printer type index by name
 *
 * Searches PRINTER_TYPES_ROLLER for a matching printer name.
 *
 * @param printer_name Name to search for (e.g., "Voron 2.4")
 * @return Index (0-based) if found, DEFAULT_PRINTER_TYPE_INDEX if not found
 */
inline int find_printer_type_index(const std::string& printer_name) {
    std::istringstream stream(PRINTER_TYPES_ROLLER);
    std::string line;
    int index = 0;

    while (std::getline(stream, line)) {
        if (line == printer_name) {
            return index;
        }
        ++index;
    }
    return DEFAULT_PRINTER_TYPE_INDEX;
}

} // namespace PrinterTypes
