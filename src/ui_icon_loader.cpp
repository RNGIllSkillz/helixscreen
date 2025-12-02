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

#include "ui_icon_loader.h"

#include <spdlog/spdlog.h>

// SDL window icon is only available when SDL is enabled
#ifdef HELIX_DISPLAY_SDL
#include "helix_icon_data.h"
#include "lvgl/src/drivers/sdl/lv_sdl_window.h"
#endif

bool ui_set_window_icon(lv_display_t* disp) {
#ifdef HELIX_DISPLAY_SDL
    spdlog::debug("[Icon] Setting window icon...");

    if (!disp) {
        spdlog::error("[Icon] Cannot set icon: display is NULL");
        return false;
    }

    // Use embedded icon data from helix_icon_data.h
    // 128x128 pixels, ARGB8888 format
    lv_sdl_window_set_icon(disp, (void*)helix_icon_128x128, 128, 128);

    spdlog::debug("[Icon] Window icon set (128x128 embedded data)");
    return true;
#else
    // Window icons are not supported on embedded displays (framebuffer/DRM)
    (void)disp;
    return false;
#endif
}
