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

#include "ui_bed_mesh.h"

#include "lvgl/lvgl.h"
#include "lvgl/src/others/xml/lv_xml.h"
#include "lvgl/src/others/xml/lv_xml_parser.h"
#include "lvgl/src/others/xml/lv_xml_widget.h"
#include "lvgl/src/others/xml/parsers/lv_xml_obj_parser.h"

#include <spdlog/spdlog.h>
#include <cstdlib>

// Canvas dimensions (600×400 RGB888 = 720,000 bytes)
#define BED_MESH_CANVAS_WIDTH 600
#define BED_MESH_CANVAS_HEIGHT 400

/**
 * Delete event handler - cleanup buffer memory
 */
static void bed_mesh_delete_cb(lv_event_t* e) {
    lv_obj_t* canvas = (lv_obj_t*)lv_event_get_target(e);
    void* buffer = lv_obj_get_user_data(canvas);

    if (buffer) {
        free(buffer);
        lv_obj_set_user_data(canvas, NULL);
        spdlog::debug("[bed_mesh] Freed buffer memory");
    }
}

/**
 * XML create handler for <bed_mesh>
 * Creates canvas widget with RGB888 buffer
 */
static void* bed_mesh_xml_create(lv_xml_parser_state_t* state, const char** attrs) {
    LV_UNUSED(attrs);

    void* parent = lv_xml_state_get_parent(state);
    lv_obj_t* canvas = lv_canvas_create((lv_obj_t*)parent);

    if (!canvas) {
        spdlog::error("[bed_mesh] Failed to create canvas");
        return NULL;
    }

    // Allocate buffer (600×400 RGB888, 24 bpp, stride=1)
    size_t buffer_size = LV_CANVAS_BUF_SIZE(BED_MESH_CANVAS_WIDTH, BED_MESH_CANVAS_HEIGHT, 24, 1);
    void* buffer = malloc(buffer_size);

    if (!buffer) {
        spdlog::error("[bed_mesh] Failed to allocate buffer ({} bytes)", buffer_size);
        lv_obj_delete(canvas);
        return NULL;
    }

    // Set canvas buffer
    lv_canvas_set_buffer(canvas, buffer, BED_MESH_CANVAS_WIDTH, BED_MESH_CANVAS_HEIGHT,
                         LV_COLOR_FORMAT_RGB888);

    // Store buffer pointer in user_data for cleanup
    lv_obj_set_user_data(canvas, buffer);

    // Register delete event handler for cleanup
    lv_obj_add_event_cb(canvas, bed_mesh_delete_cb, LV_EVENT_DELETE, NULL);

    // Set default size
    lv_obj_set_size(canvas, BED_MESH_CANVAS_WIDTH, BED_MESH_CANVAS_HEIGHT);

    spdlog::debug("[bed_mesh] Created canvas: {}x{} RGB888 ({} bytes)", BED_MESH_CANVAS_WIDTH,
                  BED_MESH_CANVAS_HEIGHT, buffer_size);

    return (void*)canvas;
}

/**
 * XML apply handler for <bed_mesh>
 * Applies standard lv_obj attributes from XML
 */
static void bed_mesh_xml_apply(lv_xml_parser_state_t* state, const char** attrs) {
    void* item = lv_xml_state_get_item(state);
    lv_obj_t* canvas = (lv_obj_t*)item;

    if (!canvas) {
        spdlog::error("[bed_mesh] NULL canvas in xml_apply");
        return;
    }

    // Apply standard lv_obj properties from XML (size, style, align, etc.)
    lv_xml_obj_apply(state, attrs);

    spdlog::trace("[bed_mesh] Applied XML attributes");
}

/**
 * Register <bed_mesh> widget with LVGL XML system
 */
void ui_bed_mesh_register(void) {
    lv_xml_register_widget("bed_mesh", bed_mesh_xml_create, bed_mesh_xml_apply);
    spdlog::info("[bed_mesh] Registered <bed_mesh> widget with XML system");
}
