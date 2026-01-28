// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize HelixScreen custom theme
 *
 * Creates a wrapper theme that delegates to LVGL default theme but overrides
 * input widget backgrounds to use a different color than cards. This gives
 * input widgets (textarea, dropdown) visual distinction from card backgrounds.
 *
 * Color computation:
 * - Dark mode: Input bg = card bg + (22, 23, 27) RGB offset (lighter)
 * - Light mode: Input bg = card bg - (22, 23, 27) RGB offset (darker)
 *
 * The theme reads all colors from globals.xml via lv_xml_get_const(), ensuring
 * no hardcoded colors in C code.
 *
 * @param display LVGL display to apply theme to
 * @param primary_color Primary theme color (from globals.xml)
 * @param secondary_color Secondary theme color (from globals.xml)
 * @param text_primary_color Primary text color for buttons/labels (theme-aware)
 * @param is_dark Dark mode flag (true = dark mode)
 * @param base_font Base font for theme
 * @param screen_bg Screen background color (from globals.xml variant)
 * @param card_bg Card background color (from globals.xml variant)
 * @param surface_control Control surface color for buttons/inputs (mode-aware)
 * @param focus_color Focus ring color for accessibility
 * @param border_color Border/outline color for slider tracks and buttons
 * @param border_radius Border radius for buttons/cards (from theme)
 * @param border_width Border width for buttons (from theme)
 * @param knob_color Color for slider/switch knobs (brighter of primary/tertiary)
 * @param accent_color Color for checkbox checkmark (more saturated of primary/secondary)
 * @return Initialized theme, or NULL on failure
 *
 * Example usage:
 * @code
 *   lv_color_t primary = theme_manager_parse_hex_color("#FF4444");
 *   lv_color_t screen_bg = theme_manager_get_color("app_bg_color");
 *   int32_t border_radius = atoi(lv_xml_get_const(NULL, "border_radius"));
 *   lv_theme_t* theme = theme_core_init(
 *       display, primary, secondary, true, font, screen_bg, card_bg, grey, border_radius
 *   );
 *   lv_display_set_theme(display, theme);
 * @endcode
 */
lv_theme_t* theme_core_init(lv_display_t* display, lv_color_t primary_color,
                            lv_color_t secondary_color, lv_color_t text_primary_color,
                            lv_color_t text_muted_color, lv_color_t text_subtle_color, bool is_dark,
                            const lv_font_t* base_font, lv_color_t screen_bg, lv_color_t card_bg,
                            lv_color_t surface_control, lv_color_t focus_color,
                            lv_color_t border_color, int32_t border_radius, int32_t border_width,
                            int32_t border_opacity, lv_color_t knob_color, lv_color_t accent_color);

/**
 * @brief Update theme colors in-place without recreating the theme
 *
 * Updates all theme style objects with new colors for runtime dark/light mode
 * switching. This modifies existing styles and calls lv_obj_report_style_change()
 * to trigger LVGL's style refresh cascade.
 *
 * Unlike theme_core_init(), this function preserves widget state and avoids
 * the overhead of theme recreation.
 *
 * @param is_dark true for dark mode colors, false for light mode
 * @param screen_bg Screen background color
 * @param card_bg Card/panel background color
 * @param surface_control Control surface color for buttons/inputs
 * @param text_primary_color Primary text color
 * @param focus_color Focus ring color for accessibility
 * @param primary_color Primary accent color (unused, kept for compatibility)
 * @param secondary_color Secondary accent color for slider/switch indicators
 * @param border_color Border color for slider tracks
 * @param knob_color Color for slider/switch knobs (brighter of secondary/tertiary)
 * @param accent_color Color for checkbox checkmark (more saturated of primary/secondary)
 */
void theme_core_update_colors(bool is_dark, lv_color_t screen_bg, lv_color_t card_bg,
                              lv_color_t surface_control, lv_color_t text_primary_color,
                              lv_color_t text_muted_color, lv_color_t text_subtle_color,
                              lv_color_t focus_color, lv_color_t primary_color,
                              lv_color_t secondary_color, lv_color_t border_color,
                              int32_t border_opacity, lv_color_t knob_color,
                              lv_color_t accent_color);

/**
 * @brief Update all theme colors for live preview
 *
 * Updates theme styles in-place without requiring restart.
 * Call lv_obj_report_style_change(NULL) after to trigger refresh.
 *
 * @param is_dark Dark mode flag
 * @param colors Array of 16 hex color strings (palette order)
 * @param border_radius Corner radius in pixels
 */
void theme_core_preview_colors(bool is_dark, const char* colors[16], int32_t border_radius,
                               int32_t border_opacity);

/**
 * @brief Get the shared card style
 *
 * Returns a pointer to the persistent card style that includes:
 * - bg_color: card_bg token
 * - bg_opa: LV_OPA_COVER
 * - border_color, border_width, border_opa
 * - radius: from border_radius parameter
 *
 * The style updates in-place when theme_core_update_colors() is called.
 *
 * @return Pointer to card style, or NULL if theme not initialized
 */
lv_style_t* theme_core_get_card_style(void);

/**
 * @brief Get the shared dialog style
 *
 * Returns a pointer to the persistent dialog style that includes:
 * - bg_color: surface_control/card_alt token
 * - bg_opa: LV_OPA_COVER
 * - radius: from border_radius parameter
 *
 * The style updates in-place when theme_core_update_colors() is called.
 *
 * @return Pointer to dialog style, or NULL if theme not initialized
 */
lv_style_t* theme_core_get_dialog_style(void);

/**
 * @brief Get the shared primary text style
 *
 * Returns a pointer to the persistent text style that includes:
 * - text_color: text_primary_color token
 *
 * The style updates in-place when theme_core_update_colors() is called.
 *
 * @return Pointer to text style, or NULL if theme not initialized
 */
lv_style_t* theme_core_get_text_style(void);

/**
 * @brief Get the shared muted text style
 *
 * Returns a pointer to the persistent muted text style that includes:
 * - text_color: text_primary_color with reduced opacity
 * - text_opa: ~70% for muted appearance
 *
 * The style updates in-place when theme_core_update_colors() is called.
 *
 * @return Pointer to muted text style, or NULL if theme not initialized
 */
lv_style_t* theme_core_get_text_muted_style(void);

/**
 * @brief Get the shared subtle text style
 *
 * Returns a pointer to the persistent subtle text style that includes:
 * - text_color: text_subtle_color (even more muted than text_muted)
 *
 * The style updates in-place when theme_core_update_colors() is called.
 *
 * @return Pointer to subtle text style, or NULL if theme not initialized
 */
lv_style_t* theme_core_get_text_subtle_style(void);

#ifdef __cplusplus
}
#endif
