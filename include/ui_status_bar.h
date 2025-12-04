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

#include "lvgl.h"
#include "printer_state.h" // For PrinterStatus and NetworkStatus enums

/**
 * @brief Status bar icon manager for HelixScreen
 *
 * Manages the persistent status icons at the top of the screen showing:
 * - Network connection status (WiFi/Ethernet)
 * - Printer connection status
 * - Active notification indicator
 */

/**
 * @brief Active notification status
 */
enum class NotificationStatus {
    NONE,    ///< No active notifications
    INFO,    ///< Info notification active
    WARNING, ///< Warning notification active
    ERROR    ///< Error notification active
};

/**
 * @brief Register status bar event callbacks
 *
 * Must be called BEFORE app_layout XML is created so LVGL can find the callbacks.
 */
void ui_status_bar_register_callbacks();

/**
 * @brief Initialize status bar subjects for XML reactive bindings
 *
 * Must be called BEFORE app_layout XML is created so XML bindings can find subjects.
 * Registers the following subjects:
 * - printer_icon_state (int: 0=ready, 1=warning, 2=error, 3=disconnected)
 * - network_icon_state (int: 0=connected, 1=connecting, 2=disconnected)
 * - notification_count (int: badge count, 0=hidden)
 * - notification_count_text (string: formatted count)
 * - notification_severity (int: 0=info, 1=warning, 2=error)
 * - overlay_backdrop_visible (int: 0=hidden, 1=visible)
 */
void ui_status_bar_init_subjects();

/**
 * @brief Initialize the status bar system
 *
 * Sets up observers on PrinterState subjects to update status bar subjects.
 * Should be called after XML is created.
 */
void ui_status_bar_init();

/**
 * @brief Set overlay backdrop visibility
 *
 * Updates the overlay_backdrop_visible subject which controls the
 * modal dimming backdrop visibility via XML binding.
 *
 * @param visible true to show backdrop, false to hide
 */
void ui_status_bar_set_backdrop_visible(bool visible);

/**
 * @brief Update network status icon
 *
 * @param status New network status
 */
void ui_status_bar_update_network(NetworkStatus status);

/**
 * @brief Update printer status icon
 *
 * @param status New printer status
 */
void ui_status_bar_update_printer(PrinterStatus status);

/**
 * @brief Update notification indicator icon
 *
 * @param status New notification status (NONE hides the icon)
 */
void ui_status_bar_update_notification(NotificationStatus status);

/**
 * @brief Update notification unread count badge
 *
 * @param count Number of unread notifications (0 hides badge)
 */
void ui_status_bar_update_notification_count(size_t count);
