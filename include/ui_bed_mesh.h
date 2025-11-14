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

#ifndef UI_BED_MESH_H
#define UI_BED_MESH_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register <bed_mesh> widget with LVGL XML system
 *
 * Creates a canvas widget (600×400 RGB888) optimized for 3D bed mesh rendering.
 * Automatically allocates buffer memory in create handler.
 *
 * Usage in XML:
 * @code{.xml}
 * <bed_mesh name="my_canvas" width="600" height="400"/>
 * @endcode
 */
void ui_bed_mesh_register(void);

#ifdef __cplusplus
}
#endif

#endif // UI_BED_MESH_H
