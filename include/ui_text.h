// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl/lvgl.h"

/**
 * Semantic text widgets for theme-aware typography
 *
 * These widgets create labels with automatic font and color styling based on
 * semantic meaning, reading from globals.xml theme constants.
 *
 * Available widgets:
 * - <text_heading>: Section headings, large text (20/26/28 + header_text)
 * - <text_body>:    Standard body text (14/18/20 + text_primary)
 * - <text_small>:   Small/helper text (12/16/18 + text_secondary)
 * - <text_xs>:      Extra-small text (10/12/14 + text_secondary) for compact metadata
 *
 * All widgets inherit standard lv_label attributes (text, width, align, etc.)
 */

/**
 * Initialize semantic text widgets for XML usage
 * Registers custom widgets: text_heading, text_body, text_small, text_xs
 *
 * Call this after globals.xml is registered but before creating UI.
 */
void ui_text_init();
