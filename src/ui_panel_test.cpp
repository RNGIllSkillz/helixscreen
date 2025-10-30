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

#include "ui_panel_test.h"
#include <spdlog/spdlog.h>
#include <cstdio>

void ui_panel_test_setup(lv_obj_t* test_panel) {
    if (!test_panel) {
        spdlog::error("[Test Panel] NULL panel");
        return;
    }

    // Get screen dimensions
    int width = lv_display_get_horizontal_resolution(lv_display_get_default());
    int height = lv_display_get_vertical_resolution(lv_display_get_default());

    // Determine screen size category
    const char* size_category;
    int switch_width, switch_height;
    int row_height;

    if (width < 600) {
        size_category = "TINY";
        switch_width = 36;
        switch_height = 18;
        row_height = 26;
    } else if (width < 900) {
        size_category = "SMALL";
        switch_width = 64;
        switch_height = 32;
        row_height = 40;
    } else {
        size_category = "LARGE";
        switch_width = 88;
        switch_height = 44;
        row_height = 56;
    }

    // Find info labels
    lv_obj_t* screen_size_label = lv_obj_find_by_name(test_panel, "screen_size_label");
    lv_obj_t* switch_size_label = lv_obj_find_by_name(test_panel, "switch_size_label");
    lv_obj_t* row_height_label = lv_obj_find_by_name(test_panel, "row_height_label");

    // Populate labels
    char buffer[128];

    if (screen_size_label) {
        snprintf(buffer, sizeof(buffer), "Screen Size: %s (%dx%d)",
                 size_category, width, height);
        lv_label_set_text(screen_size_label, buffer);
    }

    if (switch_size_label) {
        snprintf(buffer, sizeof(buffer), "Switch Size: %dx%dpx (knob padding varies)",
                 switch_width, switch_height);
        lv_label_set_text(switch_size_label, buffer);
    }

    if (row_height_label) {
        snprintf(buffer, sizeof(buffer), "Row Height: %dpx (fits switch + padding)",
                 row_height);
        lv_label_set_text(row_height_label, buffer);
    }

    spdlog::info("[Test Panel] Setup complete: {} ({}x{}), switch={}x{}, row={}px",
                 size_category, width, height, switch_width, switch_height, row_height);
}
