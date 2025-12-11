// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl/lvgl.h"

/**
 * @brief Panel indices for navigation system
 */
#define PANEL_HOME 0     ///< Home panel index
#define PANEL_CONTROLS 1 ///< Controls panel index
#define PANEL_FILAMENT 2 ///< Filament panel index
#define PANEL_SETTINGS 3 ///< Settings panel index
#define PANEL_ADVANCED 4 ///< Advanced panel index
#define PANEL_COUNT 5    ///< Total number of panels

/**
 * @brief Initialize navigation system with active panel subject
 *
 * Sets up reactive subject for panel switching and icon highlighting.
 * MUST be called BEFORE creating navigation bar XML.
 *
 * Call order: ui_navigation_init() → ui_navigation_create()
 */
void ui_navigation_init();

/**
 * @brief Create navigation bar from XML and wire up event handlers
 *
 * Creates navigation bar widget and attaches icon click handlers for
 * panel switching with reactive color updates.
 *
 * NOTE: ui_navigation_init() must be called first!
 *
 * @param parent Parent widget to attach navbar to
 * @return Created navigation bar widget
 */
lv_obj_t* ui_navigation_create(lv_obj_t* parent);

/**
 * @brief Switch to specific panel
 *
 * Triggers icon color updates via reactive subject and manages
 * panel visibility.
 *
 * @param panel_index Panel index (0-4, use PANEL_* defines)
 */
void ui_navigation_switch_to_panel(int panel_index);

/**
 * @brief Get current active panel index
 *
 * @return Active panel index (0-4)
 */
int ui_navigation_get_active_panel();

/**
 * @brief Register panel widgets for visibility management
 *
 * Stores references to panel widgets so navigation system can
 * control their visibility during panel switches.
 *
 * @param panels Array of panel widgets (size: PANEL_COUNT). NULL entries allowed for
 * not-yet-created panels.
 */
void ui_navigation_set_panels(lv_obj_t** panels);
