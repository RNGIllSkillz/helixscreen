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

#include "ui_theme.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/others/xml/lv_xml.h"
#include "lvgl/src/themes/lv_theme_private.h"  // Access to lv_theme_t complete definition
#include <spdlog/spdlog.h>
#include <cstdlib>

static lv_theme_t* current_theme = nullptr;
static bool use_dark_mode = true;
static lv_display_t* theme_display = nullptr;

// Parse hex color string "#FF4444" -> lv_color_hex(0xFF4444)
lv_color_t ui_theme_parse_color(const char* hex_str) {
    if (!hex_str || hex_str[0] != '#') {
        spdlog::error("[Theme] Invalid hex color string: {}", hex_str ? hex_str : "NULL");
        return lv_color_hex(0x000000);
    }
    uint32_t hex = strtoul(hex_str + 1, NULL, 16);
    return lv_color_hex(hex);
}

// LVGL internal theme structure for color patching
// Mimics structure from lvgl/src/themes/default/lv_theme_default.c
// NOTE: Uses LVGL private API - may need updates when upgrading LVGL versions

// Styles structure from LVGL default theme
typedef struct {
    lv_style_t scr;
    lv_style_t scrollbar;
    lv_style_t scrollbar_scrolled;
    lv_style_t card;
    lv_style_t btn;

    // Utility styles
    lv_style_t bg_color_primary;
    lv_style_t bg_color_primary_muted;
    lv_style_t bg_color_secondary;
    lv_style_t bg_color_secondary_muted;
    lv_style_t bg_color_grey;
    lv_style_t bg_color_white;
    lv_style_t pressed;
    lv_style_t disabled;
    lv_style_t pad_zero;
    lv_style_t pad_tiny;
    lv_style_t pad_small;
    lv_style_t pad_normal;
    lv_style_t pad_gap;
    lv_style_t line_space_large;
    lv_style_t text_align_center;
    lv_style_t outline_primary;
    lv_style_t outline_secondary;
    lv_style_t circle;
    lv_style_t no_radius;
    lv_style_t clip_corner;
    lv_style_t rotary_scroll;
#if LV_THEME_DEFAULT_GROW
    lv_style_t grow;
#endif
    lv_style_t transition_delayed;
    lv_style_t transition_normal;
    lv_style_t anim;
    lv_style_t anim_fast;

    // Parts
    lv_style_t knob;

#if LV_USE_ARC
    lv_style_t arc_indic;
    lv_style_t arc_indic_primary;
#endif

#if LV_USE_CHART
    lv_style_t chart_series, chart_indic, chart_bg;
#endif

#if LV_USE_DROPDOWN
    lv_style_t dropdown_list;
#endif

#if LV_USE_CHECKBOX
    lv_style_t cb_marker, cb_marker_checked;
#endif

#if LV_USE_SWITCH
    lv_style_t switch_knob;
#endif

#if LV_USE_LINE
    lv_style_t line;
#endif

#if LV_USE_TABLE
    lv_style_t table_cell;
#endif

#if LV_USE_TEXTAREA
    lv_style_t ta_cursor, ta_placeholder;
#endif

#if LV_USE_CALENDAR
    lv_style_t calendar_btnm_bg, calendar_btnm_day, calendar_header;
#endif

#if LV_USE_MENU
    lv_style_t menu_bg, menu_cont, menu_sidebar_cont, menu_main_cont, menu_page, menu_header_cont, menu_header_btn,
               menu_section, menu_pressed, menu_separator;
#endif

#if LV_USE_MSGBOX
    lv_style_t msgbox_backdrop_bg;
#endif

#if LV_USE_KEYBOARD
    lv_style_t keyboard_button_bg;
#endif
} my_theme_styles_t;

// Main theme structure - must match LVGL's internal layout
typedef struct {
    lv_theme_t base;
    uint32_t disp_size;  // disp_size_t is an enum, treat as uint32_t
    int32_t disp_dpi;
    lv_color_t color_scr;   // Screen background color
    lv_color_t color_text;  // Primary text color
    lv_color_t color_card;  // Card background color
    lv_color_t color_grey;  // Default grey color
    bool inited;
    my_theme_styles_t styles;  // Pre-computed styles used by theme_apply()
    // Note: Transitions omitted - we don't need them for color patching
} my_theme_t;

/**
 * Patch LVGL default theme colors with custom values from globals.xml
 *
 * Called after lv_theme_default_init() to override hardcoded LVGL colors
 * with user-customizable values. Accesses internal theme structure to
 * modify color fields directly.
 *
 * @param theme Theme pointer returned by lv_theme_default_init()
 * @param is_dark Whether dark mode is active (selects color variants)
 */
static void ui_theme_patch_colors(lv_theme_t* theme, bool is_dark) {
    if (!theme) {
        spdlog::error("[Theme] Cannot patch colors: NULL theme");
        return;
    }

    // Cast to internal structure to access color fields
    my_theme_t* my_theme = reinterpret_cast<my_theme_t*>(theme);

    // Read custom color variants from globals.xml
    const char* screen_bg_light = lv_xml_get_const(nullptr, "app_bg_color_light");
    const char* screen_bg_dark = lv_xml_get_const(nullptr, "app_bg_color_dark");
    const char* card_bg_light = lv_xml_get_const(nullptr, "card_bg_light");
    const char* card_bg_dark = lv_xml_get_const(nullptr, "card_bg_dark");
    const char* theme_grey_light = lv_xml_get_const(nullptr, "theme_grey_light");
    const char* theme_grey_dark = lv_xml_get_const(nullptr, "theme_grey_dark");

    // Validate all color constants exist
    if (!screen_bg_light || !screen_bg_dark || !card_bg_light || !card_bg_dark ||
        !theme_grey_light || !theme_grey_dark) {
        spdlog::error("[Theme] Failed to read custom theme color constants from globals.xml");
        return;
    }

    // Select theme-appropriate color variants
    const char* screen_bg_str = is_dark ? screen_bg_dark : screen_bg_light;
    const char* card_bg_str = is_dark ? card_bg_dark : card_bg_light;
    const char* theme_grey_str = is_dark ? theme_grey_dark : theme_grey_light;

    // Parse colors and apply to theme structure
    lv_color_t screen_bg = ui_theme_parse_color(screen_bg_str);
    lv_color_t card_bg = ui_theme_parse_color(card_bg_str);
    lv_color_t theme_grey = ui_theme_parse_color(theme_grey_str);

    my_theme->color_scr = screen_bg;
    my_theme->color_card = card_bg;
    my_theme->color_grey = theme_grey;

    // CRITICAL: Update ALL pre-computed styles that were baked with old colors
    // LVGL's style_init() copies colors into styles during theme_init, so we must
    // update every style that references color_scr, color_card, or color_grey

    // Styles using color_scr (1 style):
    lv_style_set_bg_color(&my_theme->styles.scr, screen_bg);

    // Styles using color_card (5 styles):
    lv_style_set_bg_color(&my_theme->styles.card, card_bg);
    lv_style_set_bg_color(&my_theme->styles.bg_color_white, card_bg);
    lv_style_set_bg_color(&my_theme->styles.cb_marker, card_bg);
    lv_style_set_bg_color(&my_theme->styles.menu_section, card_bg);
    lv_style_set_bg_color(&my_theme->styles.calendar_btnm_day, card_bg);

    // Styles using color_grey (2 styles):
    lv_style_set_bg_color(&my_theme->styles.btn, theme_grey);
    lv_style_set_bg_color(&my_theme->styles.bg_color_grey, theme_grey);

    spdlog::info("[Theme] Patched theme colors: screen={} (0x{:06X}), card={} (0x{:06X}), grey={} (0x{:06X}) ({} mode)",
                 screen_bg_str, lv_color_to_u32(screen_bg) & 0xFFFFFF,
                 card_bg_str, lv_color_to_u32(card_bg) & 0xFFFFFF,
                 theme_grey_str, lv_color_to_u32(theme_grey) & 0xFFFFFF,
                 is_dark ? "dark" : "light");
}

void ui_theme_register_responsive_padding(lv_display_t* display) {
    // Use custom breakpoints optimized for our hardware: max(hor_res, ver_res)
    int32_t hor_res = lv_display_get_horizontal_resolution(display);
    int32_t ver_res = lv_display_get_vertical_resolution(display);
    int32_t greater_res = LV_MAX(hor_res, ver_res);

    // Determine size suffix for variant lookup (using centralized breakpoints)
    const char* size_suffix;
    const char* size_label;

    if (greater_res <= UI_BREAKPOINT_SMALL_MAX) {  // ≤480: 480x320
        size_suffix = "_small";
        size_label = "SMALL";
    } else if (greater_res <= UI_BREAKPOINT_MEDIUM_MAX) {  // 481-800: 800x480, up to 800x600
        size_suffix = "_medium";
        size_label = "MEDIUM";
    } else {  // >800: 1024x600, 1280x720+
        size_suffix = "_large";
        size_label = "LARGE";
    }

    // Read size-specific variants from XML
    char variant_name[64];

    snprintf(variant_name, sizeof(variant_name), "padding_normal%s", size_suffix);
    const char* padding_normal = lv_xml_get_const(NULL, variant_name);

    snprintf(variant_name, sizeof(variant_name), "padding_small%s", size_suffix);
    const char* padding_small = lv_xml_get_const(NULL, variant_name);

    snprintf(variant_name, sizeof(variant_name), "padding_tiny%s", size_suffix);
    const char* padding_tiny = lv_xml_get_const(NULL, variant_name);

    snprintf(variant_name, sizeof(variant_name), "gap_normal%s", size_suffix);
    const char* gap_normal = lv_xml_get_const(NULL, variant_name);

    // Validate that variants were found
    if (!padding_normal || !padding_small || !padding_tiny || !gap_normal) {
        spdlog::error("[Theme] Failed to read padding variants for size: {} (normal={}, small={}, tiny={}, gap={})",
                      size_label,
                      padding_normal ? "found" : "NULL",
                      padding_small ? "found" : "NULL",
                      padding_tiny ? "found" : "NULL",
                      gap_normal ? "found" : "NULL");
        return;
    }

    // Register active constants (override defaults in globals scope)
    lv_xml_component_scope_t* scope = lv_xml_component_get_scope("globals");
    if (scope) {
        lv_xml_register_const(scope, "padding_normal", padding_normal);
        lv_xml_register_const(scope, "padding_small", padding_small);
        lv_xml_register_const(scope, "padding_tiny", padding_tiny);
        lv_xml_register_const(scope, "gap_normal", gap_normal);

        spdlog::info("[Theme] Responsive padding: {} ({}px) - normal={}, small={}, tiny={}, gap={}",
                     size_label, greater_res, padding_normal, padding_small, padding_tiny, gap_normal);
    } else {
        spdlog::warn("[Theme] Failed to get globals scope for padding constants");
    }
}

void ui_theme_init(lv_display_t* display, bool use_dark_mode_param) {
    theme_display = display;
    use_dark_mode = use_dark_mode_param;

    // Override runtime theme constants based on light/dark mode preference
    lv_xml_component_scope_t* scope = lv_xml_component_get_scope("globals");
    if (!scope) {
        spdlog::critical("[Theme] FATAL: Failed to get globals scope for runtime constant registration");
        std::exit(EXIT_FAILURE);
    }

    // Read light/dark color variants from XML (MUST exist - fail-fast if missing)
    const char* app_bg_light = lv_xml_get_const(NULL, "app_bg_color_light");
    const char* app_bg_dark = lv_xml_get_const(NULL, "app_bg_color_dark");
    const char* text_primary_light = lv_xml_get_const(NULL, "text_primary_light");
    const char* text_primary_dark = lv_xml_get_const(NULL, "text_primary_dark");
    const char* header_text_light = lv_xml_get_const(NULL, "header_text_light");
    const char* header_text_dark = lv_xml_get_const(NULL, "header_text_dark");

    // Validate ALL required color variants exist
    if (!app_bg_light || !app_bg_dark) {
        spdlog::critical("[Theme] FATAL: Missing app_bg_color_light/dark in globals.xml");
        std::exit(EXIT_FAILURE);
    }
    if (!text_primary_light || !text_primary_dark) {
        spdlog::critical("[Theme] FATAL: Missing text_primary_light/dark in globals.xml");
        std::exit(EXIT_FAILURE);
    }
    if (!header_text_light || !header_text_dark) {
        spdlog::critical("[Theme] FATAL: Missing header_text_light/dark in globals.xml");
        std::exit(EXIT_FAILURE);
    }

    // Register runtime constants based on theme preference
    const char* selected_bg = use_dark_mode ? app_bg_dark : app_bg_light;
    lv_xml_register_const(scope, "app_bg_color", selected_bg);
    spdlog::debug("[Theme] Registered app_bg_color={} for {} mode",
                  selected_bg, use_dark_mode ? "dark" : "light");

    const char* selected_text = use_dark_mode ? text_primary_dark : text_primary_light;
    lv_xml_register_const(scope, "text_primary", selected_text);
    spdlog::debug("[Theme] Registered text_primary={} for {} mode",
                  selected_text, use_dark_mode ? "dark" : "light");

    const char* selected_header = use_dark_mode ? header_text_dark : header_text_light;
    lv_xml_register_const(scope, "header_text_color", selected_header);
    spdlog::debug("[Theme] Registered header_text_color={} for {} mode",
                  selected_header, use_dark_mode ? "dark" : "light");

    spdlog::debug("[Theme] Runtime constants set for {} mode", use_dark_mode ? "dark" : "light");

    // Read colors from globals.xml
    const char* primary_str = lv_xml_get_const(NULL, "primary_color");
    const char* secondary_str = lv_xml_get_const(NULL, "secondary_color");

    if (!primary_str || !secondary_str) {
        spdlog::error("[Theme] Failed to read color constants from globals.xml");
        return;
    }

    lv_color_t primary_color = ui_theme_parse_color(primary_str);
    lv_color_t secondary_color = ui_theme_parse_color(secondary_str);

    // Read base font from globals.xml
    const char* font_body_name = lv_xml_get_const(NULL, "font_body");
    const lv_font_t* base_font = lv_xml_get_font(NULL, font_body_name);
    if (!base_font) {
        spdlog::warn("[Theme] Failed to get font '{}', using montserrat_16", font_body_name);
        base_font = &lv_font_montserrat_16;
    }

    // Initialize LVGL default theme
    current_theme = lv_theme_default_init(
        display,
        primary_color,
        secondary_color,
        use_dark_mode,
        base_font
    );

    if (current_theme) {
        // Apply custom theme colors from globals.xml
        ui_theme_patch_colors(current_theme, use_dark_mode);

        // DEBUG: Verify patched values actually stuck
        my_theme_t* my_theme = reinterpret_cast<my_theme_t*>(current_theme);
        spdlog::debug("[Theme] After patching - color_card in theme structure: 0x{:06X}",
                      lv_color_to_u32(my_theme->color_card) & 0xFFFFFF);

        lv_display_set_theme(display, current_theme);
        spdlog::info("[Theme] Initialized: {} mode, primary={}, secondary={}, base_font={}",
                     use_dark_mode ? "dark" : "light", primary_str, secondary_str, font_body_name);

        // Register responsive padding constants AFTER theme init
        ui_theme_register_responsive_padding(display);
    } else {
        spdlog::error("[Theme] Failed to initialize default theme");
    }
}

void ui_theme_toggle_dark_mode() {
    if (!theme_display) {
        spdlog::error("[Theme] Cannot toggle: theme not initialized");
        return;
    }

    bool new_use_dark_mode = !use_dark_mode;
    spdlog::info("[Theme] Toggling to {} mode", new_use_dark_mode ? "dark" : "light");

    ui_theme_init(theme_display, new_use_dark_mode);
    lv_obj_invalidate(lv_screen_active());
}

bool ui_theme_is_dark_mode() {
    return use_dark_mode;
}

/**
 * Get theme-appropriate color variant
 *
 * Looks up {base_name}_light and {base_name}_dark from globals.xml,
 * selects the appropriate one based on current theme mode, and returns
 * the parsed lv_color_t.
 *
 * @param base_name Color constant base name (e.g., "app_bg_color", "card_bg")
 * @return Parsed color, or black (0x000000) if not found
 *
 * Example:
 *   lv_color_t bg = ui_theme_get_color("app_bg_color");
 *   // Returns app_bg_color_light in light mode, app_bg_color_dark in dark mode
 */
lv_color_t ui_theme_get_color(const char* base_name) {
    if (!base_name) {
        spdlog::error("[Theme] ui_theme_get_color: NULL base_name");
        return lv_color_hex(0x000000);
    }

    // Construct variant names: {base_name}_light and {base_name}_dark
    char light_name[128];
    char dark_name[128];
    snprintf(light_name, sizeof(light_name), "%s_light", base_name);
    snprintf(dark_name, sizeof(dark_name), "%s_dark", base_name);

    // Look up color strings from globals.xml
    const char* light_str = lv_xml_get_const(nullptr, light_name);
    const char* dark_str = lv_xml_get_const(nullptr, dark_name);

    if (!light_str || !dark_str) {
        spdlog::error("[Theme] Color variant not found: {} (light={}, dark={})",
                      base_name, light_str ? "found" : "missing", dark_str ? "found" : "missing");
        return lv_color_hex(0x000000);
    }

    // Select appropriate variant based on theme mode
    const char* selected_str = use_dark_mode ? dark_str : light_str;
    lv_color_t color = ui_theme_parse_color(selected_str);

    spdlog::debug("[Theme] ui_theme_get_color({}) = {} (0x{:06X}) ({} mode)",
                  base_name, selected_str, lv_color_to_u32(color) & 0xFFFFFF,
                  use_dark_mode ? "dark" : "light");

    return color;
}

/**
 * Apply theme-appropriate background color to object
 *
 * Convenience wrapper that gets the color variant and applies it to the object.
 *
 * @param obj LVGL object to apply color to
 * @param base_name Color constant base name (e.g., "app_bg_color", "card_bg")
 * @param part Style part to apply to (default: LV_PART_MAIN)
 *
 * Example:
 *   ui_theme_apply_bg_color(screen, "app_bg_color", LV_PART_MAIN);
 *   // Applies app_bg_color_light/dark depending on theme mode
 */
void ui_theme_apply_bg_color(lv_obj_t* obj, const char* base_name, lv_part_t part) {
    if (!obj) {
        spdlog::error("[Theme] ui_theme_apply_bg_color: NULL object");
        return;
    }

    lv_color_t color = ui_theme_get_color(base_name);
    lv_obj_set_style_bg_color(obj, color, part);

    spdlog::info("[Theme] Applied background color {} (0x{:06X}) to object (part={})",
                 base_name, lv_color_to_u32(color) & 0xFFFFFF, static_cast<int>(part));
}

/**
 * Get font line height in pixels
 *
 * Returns the total vertical space a line of text will occupy for the given font.
 * This includes ascender, descender, and line gap. Useful for calculating layout
 * heights before widgets are created.
 *
 * @param font Font to query (e.g., UI_FONT_HEADING, &lv_font_montserrat_16)
 * @return Line height in pixels, or 0 if font is NULL
 *
 * Examples:
 *   int32_t heading_h = ui_theme_get_font_height(UI_FONT_HEADING);  // ~24px
 *   int32_t body_h = ui_theme_get_font_height(UI_FONT_BODY);        // ~20px
 *   int32_t small_h = ui_theme_get_font_height(UI_FONT_SMALL);      // ~15px
 *
 *   // Calculate total height for multi-line layout
 *   int32_t total = ui_theme_get_font_height(UI_FONT_HEADING) +
 *                   (ui_theme_get_font_height(UI_FONT_BODY) * 3) +
 *                   (4 * 8);  // 4 gaps of 8px padding
 */
int32_t ui_theme_get_font_height(const lv_font_t* font) {
    if (!font) {
        spdlog::warn("[Theme] ui_theme_get_font_height: NULL font pointer");
        return 0;
    }

    int32_t height = lv_font_get_line_height(font);

    spdlog::trace("[Theme] Font line height: {}px", height);

    return height;
}
