// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_theme_core_styles.cpp
 * @brief Tests for theme_core style getters (Phase 1.1 of reactive theming)
 *
 * These tests define the API contract for shared style getters in theme_core.
 * The getters return pointers to persistent lv_style_t objects that:
 * 1. Are non-null after theme initialization
 * 2. Have appropriate style properties set (bg_color for surfaces, text_color for text)
 * 3. Update in-place when theme_core_update_colors() is called (reactive behavior)
 *
 * Tests are written to FAIL until the implementation is complete.
 */

#include "../lvgl_ui_test_fixture.h"
#include "theme_core.h"
#include "theme_manager.h"

#include "../catch_amalgamated.hpp"

// ============================================================================
// Card Style Getter Tests
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: card style getter returns valid style",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_card_style();
    REQUIRE(style != nullptr);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: card style has background color set",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_card_style();
    REQUIRE(style != nullptr);

    // Card style should have bg_color property set
    lv_style_value_t value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_BG_COLOR, &value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    // Verify it's not black (some meaningful color is set)
    // Theme initializes in light mode, so card_bg should not be (0,0,0)
    uint32_t color_rgb = lv_color_to_u32(value.color) & 0x00FFFFFF;
    INFO("Card bg_color RGB: 0x" << std::hex << color_rgb);
    // Just verify a color is set - don't hardcode expected values
    // (the actual color depends on the theme configuration)
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: card style has background opacity set",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_card_style();
    REQUIRE(style != nullptr);

    // Card style should have bg_opa set for visibility
    lv_style_value_t value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_BG_OPA, &value);

    // If bg_opa is set, it should be fully opaque or close to it
    if (res == LV_STYLE_RES_FOUND) {
        REQUIRE(value.num >= LV_OPA_50); // At least 50% opacity
    }
    // Note: If not found, widget will inherit default (which is typically opaque)
}

// ============================================================================
// Dialog Style Getter Tests
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: dialog style getter returns valid style",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_dialog_style();
    REQUIRE(style != nullptr);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: dialog style has background color set",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_dialog_style();
    REQUIRE(style != nullptr);

    // Dialog style should have bg_color property set
    lv_style_value_t value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_BG_COLOR, &value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    uint32_t color_rgb = lv_color_to_u32(value.color) & 0x00FFFFFF;
    INFO("Dialog bg_color RGB: 0x" << std::hex << color_rgb);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: dialog style is distinct pointer from card style",
                 "[theme-core]") {
    lv_style_t* card_style = theme_core_get_card_style();
    lv_style_t* dialog_style = theme_core_get_dialog_style();

    REQUIRE(card_style != nullptr);
    REQUIRE(dialog_style != nullptr);

    // Should be different style objects (different use cases may need different styling)
    REQUIRE(card_style != dialog_style);
}

// ============================================================================
// Text Style Getter Tests
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: text style getter returns valid style",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_text_style();
    REQUIRE(style != nullptr);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: text style has text color set", "[theme-core]") {
    lv_style_t* style = theme_core_get_text_style();
    REQUIRE(style != nullptr);

    // Text style should have text_color property set
    lv_style_value_t value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    uint32_t color_rgb = lv_color_to_u32(value.color) & 0x00FFFFFF;
    INFO("Text color RGB: 0x" << std::hex << color_rgb);
}

// ============================================================================
// Muted Text Style Getter Tests
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: muted text style getter returns valid style",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_text_muted_style();
    REQUIRE(style != nullptr);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: muted text style has text color set",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_text_muted_style();
    REQUIRE(style != nullptr);

    // Muted text style should have text_color property set
    lv_style_value_t value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    uint32_t color_rgb = lv_color_to_u32(value.color) & 0x00FFFFFF;
    INFO("Muted text color RGB: 0x" << std::hex << color_rgb);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: muted text style is distinct from primary text",
                 "[theme-core]") {
    lv_style_t* text_style = theme_core_get_text_style();
    lv_style_t* muted_style = theme_core_get_text_muted_style();

    REQUIRE(text_style != nullptr);
    REQUIRE(muted_style != nullptr);

    // Should be different style objects
    REQUIRE(text_style != muted_style);
}

// ============================================================================
// Subtle Text Style Getter Tests
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: subtle text style getter returns valid style",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_text_subtle_style();
    REQUIRE(style != nullptr);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: subtle text style has text color set",
                 "[theme-core]") {
    lv_style_t* style = theme_core_get_text_subtle_style();
    REQUIRE(style != nullptr);

    // Subtle text style should have text_color property set
    lv_style_value_t value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    uint32_t color_rgb = lv_color_to_u32(value.color) & 0x00FFFFFF;
    INFO("Subtle text color RGB: 0x" << std::hex << color_rgb);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: subtle text style is distinct from muted text",
                 "[theme-core]") {
    lv_style_t* muted_style = theme_core_get_text_muted_style();
    lv_style_t* subtle_style = theme_core_get_text_subtle_style();

    REQUIRE(muted_style != nullptr);
    REQUIRE(subtle_style != nullptr);

    // Should be different style objects
    REQUIRE(muted_style != subtle_style);
}

// ============================================================================
// Style Consistency Tests
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: style getters return same pointer on repeat calls",
                 "[theme-core]") {
    // Style pointers should be stable - multiple calls return same object
    lv_style_t* card1 = theme_core_get_card_style();
    lv_style_t* card2 = theme_core_get_card_style();
    REQUIRE(card1 == card2);

    lv_style_t* dialog1 = theme_core_get_dialog_style();
    lv_style_t* dialog2 = theme_core_get_dialog_style();
    REQUIRE(dialog1 == dialog2);

    lv_style_t* text1 = theme_core_get_text_style();
    lv_style_t* text2 = theme_core_get_text_style();
    REQUIRE(text1 == text2);

    lv_style_t* muted1 = theme_core_get_text_muted_style();
    lv_style_t* muted2 = theme_core_get_text_muted_style();
    REQUIRE(muted1 == muted2);

    lv_style_t* subtle1 = theme_core_get_text_subtle_style();
    lv_style_t* subtle2 = theme_core_get_text_subtle_style();
    REQUIRE(subtle1 == subtle2);
}

// ============================================================================
// Reactive Update Tests - CRITICAL for reactive theming
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: card style updates on theme change",
                 "[theme-core][reactive]") {
    lv_style_t* style = theme_core_get_card_style();
    REQUIRE(style != nullptr);

    // Get initial color
    lv_style_value_t before_value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_BG_COLOR, &before_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t before = before_value.color;

    // Switch theme mode - use different colors for dark mode
    // These are typical dark mode colors
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, // is_dark
                             dark_screen_bg, dark_card_bg, dark_surface, dark_text, dark_text_muted,
                             dark_text_subtle, dark_focus, dark_primary, dark_border);

    // Get color after update
    lv_style_value_t after_value;
    res = lv_style_get_prop(style, LV_STYLE_BG_COLOR, &after_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t after = after_value.color;

    // Style should have updated in-place (same pointer, different value)
    // The test fixture initializes in light mode, so switching to dark should change the color
    REQUIRE_FALSE(lv_color_eq(before, after));

    INFO("Before: 0x" << std::hex << (lv_color_to_u32(before) & 0x00FFFFFF));
    INFO("After: 0x" << std::hex << (lv_color_to_u32(after) & 0x00FFFFFF));
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: dialog style updates on theme change",
                 "[theme-core][reactive]") {
    lv_style_t* style = theme_core_get_dialog_style();
    REQUIRE(style != nullptr);

    // Get initial color
    lv_style_value_t before_value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_BG_COLOR, &before_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t before = before_value.color;

    // Switch to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    // Get color after update
    lv_style_value_t after_value;
    res = lv_style_get_prop(style, LV_STYLE_BG_COLOR, &after_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t after = after_value.color;

    REQUIRE_FALSE(lv_color_eq(before, after));
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: text style updates on theme change",
                 "[theme-core][reactive]") {
    lv_style_t* style = theme_core_get_text_style();
    REQUIRE(style != nullptr);

    // Get initial color
    lv_style_value_t before_value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &before_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t before = before_value.color;

    // Switch to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    // Get color after update
    lv_style_value_t after_value;
    res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &after_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t after = after_value.color;

    // Light mode text is dark, dark mode text is light - should differ
    REQUIRE_FALSE(lv_color_eq(before, after));
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: muted text style updates on theme change",
                 "[theme-core][reactive]") {
    lv_style_t* style = theme_core_get_text_muted_style();
    REQUIRE(style != nullptr);

    // Get initial color
    lv_style_value_t before_value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &before_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t before = before_value.color;

    // Switch to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    // Get color after update
    lv_style_value_t after_value;
    res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &after_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t after = after_value.color;

    REQUIRE_FALSE(lv_color_eq(before, after));
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: subtle text style updates on theme change",
                 "[theme-core][reactive]") {
    lv_style_t* style = theme_core_get_text_subtle_style();
    REQUIRE(style != nullptr);

    // Get initial color
    lv_style_value_t before_value;
    lv_style_res_t res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &before_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t before = before_value.color;

    // Switch to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    // Get color after update
    lv_style_value_t after_value;
    res = lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &after_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);
    lv_color_t after = after_value.color;

    REQUIRE_FALSE(lv_color_eq(before, after));
}

// ============================================================================
// Widget Integration Test - Verify styles work when applied to widgets
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: card style can be applied to widget",
                 "[theme-core][integration]") {
    lv_style_t* style = theme_core_get_card_style();
    REQUIRE(style != nullptr);

    // Create a test widget
    lv_obj_t* card = lv_obj_create(test_screen());
    REQUIRE(card != nullptr);

    // Apply the shared style - should not crash
    lv_obj_add_style(card, style, LV_PART_MAIN);

    // Widget should now have the style's background color
    lv_color_t widget_bg = lv_obj_get_style_bg_color(card, LV_PART_MAIN);

    // Get expected color from style
    lv_style_value_t value;
    lv_style_get_prop(style, LV_STYLE_BG_COLOR, &value);

    REQUIRE(lv_color_eq(widget_bg, value.color));

    lv_obj_delete(card);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: text style can be applied to label",
                 "[theme-core][integration]") {
    lv_style_t* style = theme_core_get_text_style();
    REQUIRE(style != nullptr);

    // Create a test label
    lv_obj_t* label = lv_label_create(test_screen());
    REQUIRE(label != nullptr);
    lv_label_set_text(label, "Test Label");

    // Apply the shared style
    lv_obj_add_style(label, style, LV_PART_MAIN);

    // Label should now have the style's text color
    lv_color_t label_color = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Get expected color from style
    lv_style_value_t value;
    lv_style_get_prop(style, LV_STYLE_TEXT_COLOR, &value);

    REQUIRE(lv_color_eq(label_color, value.color));

    lv_obj_delete(label);
}

TEST_CASE_METHOD(LVGLUITestFixture, "theme_core: widget updates when shared style changes",
                 "[theme-core][integration][reactive]") {
    lv_style_t* style = theme_core_get_card_style();
    REQUIRE(style != nullptr);

    // Create widget and apply style
    lv_obj_t* card = lv_obj_create(test_screen());
    lv_obj_add_style(card, style, LV_PART_MAIN);

    // Get initial color
    lv_color_t before = lv_obj_get_style_bg_color(card, LV_PART_MAIN);

    // Update theme colors
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    // Trigger LVGL style refresh (this is what theme_core_update_colors should do internally)
    lv_obj_report_style_change(nullptr);

    // Get color after update
    lv_color_t after = lv_obj_get_style_bg_color(card, LV_PART_MAIN);

    // Widget should reflect the new style color
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(card);
}

// ============================================================================
// ui_card Widget Reactive Style Tests - Phase 1.2
// ============================================================================
// These tests verify that ui_card widgets update their appearance when the
// theme changes. They should FAIL with the current implementation because
// ui_card uses inline styles (lv_obj_set_style_bg_color) that don't respond
// to theme changes.
//
// The fix (Phase 1.2 IMPL) will make ui_card use the shared card_style_ from
// theme_core, which updates in-place when theme_core_update_colors() is called.
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "ui_card: background color updates on theme change",
                 "[reactive-card]") {
    // Create ui_card widget via XML
    lv_obj_t* card = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_card", nullptr));
    REQUIRE(card != nullptr);

    // Get initial bg_color from the card widget
    lv_color_t before = lv_obj_get_style_bg_color(card, LV_PART_MAIN);

    // Log initial color for debugging
    uint32_t before_rgb = lv_color_to_u32(before) & 0x00FFFFFF;
    INFO("Initial card bg_color: 0x" << std::hex << before_rgb);

    // Update theme colors to dark mode (significantly different colors)
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E); // Notably different from light mode
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, // is_dark
                             dark_screen_bg, dark_card_bg, dark_surface, dark_text, dark_text_muted,
                             dark_text_subtle, dark_focus, dark_primary, dark_border);

    // Force LVGL style refresh cascade
    lv_obj_report_style_change(nullptr);

    // Get updated color from the card
    lv_color_t after = lv_obj_get_style_bg_color(card, LV_PART_MAIN);

    // Log updated color for debugging
    uint32_t after_rgb = lv_color_to_u32(after) & 0x00FFFFFF;
    INFO("After theme change bg_color: 0x" << std::hex << after_rgb);

    // This assertion will FAIL with current implementation because ui_card uses
    // inline styles (lv_obj_set_style_bg_color) that don't respond to theme changes.
    // Once ui_card is updated to use the shared card_style_, this will pass.
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(card);
}

TEST_CASE_METHOD(LVGLUITestFixture, "ui_card: uses card_bg token color initially",
                 "[reactive-card]") {
    // Create ui_card widget
    lv_obj_t* card = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_card", nullptr));
    REQUIRE(card != nullptr);

    // Get the shared card style from theme_core (if available)
    lv_style_t* shared_style = theme_core_get_card_style();
    REQUIRE(shared_style != nullptr);

    // Get expected color from the shared style
    lv_style_value_t expected_value;
    lv_style_res_t res = lv_style_get_prop(shared_style, LV_STYLE_BG_COLOR, &expected_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    // Get actual color from ui_card
    lv_color_t actual = lv_obj_get_style_bg_color(card, LV_PART_MAIN);

    // Log colors for debugging
    uint32_t expected_rgb = lv_color_to_u32(expected_value.color) & 0x00FFFFFF;
    uint32_t actual_rgb = lv_color_to_u32(actual) & 0x00FFFFFF;
    INFO("Expected (from shared style): 0x" << std::hex << expected_rgb);
    INFO("Actual (from ui_card): 0x" << std::hex << actual_rgb);

    // Both should be the same card_bg color
    // Note: This may pass since both read from theme_manager_get_color("card_bg")
    // at initialization time. The real test is whether it updates on theme change.
    REQUIRE(lv_color_eq(actual, expected_value.color));

    lv_obj_delete(card);
}

TEST_CASE_METHOD(LVGLUITestFixture, "ui_card: multiple cards update together on theme change",
                 "[reactive-card]") {
    // Create multiple ui_card widgets
    lv_obj_t* card1 = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_card", nullptr));
    lv_obj_t* card2 = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_card", nullptr));
    lv_obj_t* card3 = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_card", nullptr));

    REQUIRE(card1 != nullptr);
    REQUIRE(card2 != nullptr);
    REQUIRE(card3 != nullptr);

    // Get initial colors
    lv_color_t before1 = lv_obj_get_style_bg_color(card1, LV_PART_MAIN);
    lv_color_t before2 = lv_obj_get_style_bg_color(card2, LV_PART_MAIN);
    lv_color_t before3 = lv_obj_get_style_bg_color(card3, LV_PART_MAIN);

    // All cards should have the same initial color
    REQUIRE(lv_color_eq(before1, before2));
    REQUIRE(lv_color_eq(before2, before3));

    // Update to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    lv_obj_report_style_change(nullptr);

    // Get colors after theme change
    lv_color_t after1 = lv_obj_get_style_bg_color(card1, LV_PART_MAIN);
    lv_color_t after2 = lv_obj_get_style_bg_color(card2, LV_PART_MAIN);
    lv_color_t after3 = lv_obj_get_style_bg_color(card3, LV_PART_MAIN);

    // All cards should still have the same color (consistency)
    REQUIRE(lv_color_eq(after1, after2));
    REQUIRE(lv_color_eq(after2, after3));

    // And the color should have changed from before (reactivity)
    // This will FAIL with current inline style implementation
    REQUIRE_FALSE(lv_color_eq(before1, after1));

    lv_obj_delete(card1);
    lv_obj_delete(card2);
    lv_obj_delete(card3);
}

TEST_CASE_METHOD(LVGLUITestFixture, "ui_card: style matches shared card_style after theme change",
                 "[reactive-card]") {
    // Create ui_card widget
    lv_obj_t* card = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_card", nullptr));
    REQUIRE(card != nullptr);

    // Get the shared card style
    lv_style_t* shared_style = theme_core_get_card_style();
    REQUIRE(shared_style != nullptr);

    // Update to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    lv_obj_report_style_change(nullptr);

    // Get the updated color from the shared style
    lv_style_value_t style_value;
    lv_style_res_t res = lv_style_get_prop(shared_style, LV_STYLE_BG_COLOR, &style_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    // Get the actual color from the ui_card widget
    lv_color_t card_color = lv_obj_get_style_bg_color(card, LV_PART_MAIN);

    // Log for debugging
    uint32_t style_rgb = lv_color_to_u32(style_value.color) & 0x00FFFFFF;
    uint32_t card_rgb = lv_color_to_u32(card_color) & 0x00FFFFFF;
    INFO("Shared style bg_color: 0x" << std::hex << style_rgb);
    INFO("ui_card actual bg_color: 0x" << std::hex << card_rgb);

    // The ui_card should have the same color as the shared style after update
    // This will FAIL until ui_card uses lv_obj_add_style() with the shared style
    REQUIRE(lv_color_eq(card_color, style_value.color));

    lv_obj_delete(card);
}

// ============================================================================
// ui_dialog Widget Reactive Style Tests - Phase 1.3
// ============================================================================
// These tests verify that ui_dialog widgets update their appearance when the
// theme changes. They should FAIL with the current implementation because
// ui_dialog uses inline styles (lv_obj_set_style_bg_color) that don't respond
// to theme changes.
//
// The fix (Phase 1.3 IMPL) will make ui_dialog use the shared dialog_style_
// from theme_core, which updates in-place when theme_core_update_colors() is
// called.
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "ui_dialog: background color updates on theme change",
                 "[reactive-dialog]") {
    // Create ui_dialog widget via XML
    lv_obj_t* dialog = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_dialog", nullptr));
    REQUIRE(dialog != nullptr);

    // Get initial bg_color from the dialog widget
    lv_color_t before = lv_obj_get_style_bg_color(dialog, LV_PART_MAIN);

    // Log initial color for debugging
    uint32_t before_rgb = lv_color_to_u32(before) & 0x00FFFFFF;
    INFO("Initial dialog bg_color: 0x" << std::hex << before_rgb);

    // Update theme colors to dark mode (significantly different colors)
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D); // Notably different from light mode
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, // is_dark
                             dark_screen_bg, dark_card_bg, dark_surface, dark_text, dark_text_muted,
                             dark_text_subtle, dark_focus, dark_primary, dark_border);

    // Force LVGL style refresh cascade
    lv_obj_report_style_change(nullptr);

    // Get updated color from the dialog
    lv_color_t after = lv_obj_get_style_bg_color(dialog, LV_PART_MAIN);

    // Log updated color for debugging
    uint32_t after_rgb = lv_color_to_u32(after) & 0x00FFFFFF;
    INFO("After theme change bg_color: 0x" << std::hex << after_rgb);

    // This assertion will FAIL with current implementation because ui_dialog uses
    // inline styles (lv_obj_set_style_bg_color) that don't respond to theme changes.
    // Once ui_dialog is updated to use the shared dialog_style_, this will pass.
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(dialog);
}

TEST_CASE_METHOD(LVGLUITestFixture,
                 "ui_dialog: style matches shared dialog_style after theme change",
                 "[reactive-dialog]") {
    // Create ui_dialog widget
    lv_obj_t* dialog = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_dialog", nullptr));
    REQUIRE(dialog != nullptr);

    // Get the shared dialog style
    lv_style_t* shared_style = theme_core_get_dialog_style();
    REQUIRE(shared_style != nullptr);

    // Update to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    lv_obj_report_style_change(nullptr);

    // Get the updated color from the shared style
    lv_style_value_t style_value;
    lv_style_res_t res = lv_style_get_prop(shared_style, LV_STYLE_BG_COLOR, &style_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    // Get the actual color from the ui_dialog widget
    lv_color_t dialog_color = lv_obj_get_style_bg_color(dialog, LV_PART_MAIN);

    // Log for debugging
    uint32_t style_rgb = lv_color_to_u32(style_value.color) & 0x00FFFFFF;
    uint32_t dialog_rgb = lv_color_to_u32(dialog_color) & 0x00FFFFFF;
    INFO("Shared dialog_style bg_color: 0x" << std::hex << style_rgb);
    INFO("ui_dialog actual bg_color: 0x" << std::hex << dialog_rgb);

    // The ui_dialog should have the same color as the shared style after update
    // This will FAIL until ui_dialog uses lv_obj_add_style() with the shared dialog_style_
    REQUIRE(lv_color_eq(dialog_color, style_value.color));

    lv_obj_delete(dialog);
}

TEST_CASE_METHOD(LVGLUITestFixture, "ui_dialog: uses card_alt token color initially",
                 "[reactive-dialog]") {
    // Create ui_dialog widget
    lv_obj_t* dialog = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_dialog", nullptr));
    REQUIRE(dialog != nullptr);

    // Get the shared dialog style from theme_core (if available)
    lv_style_t* shared_style = theme_core_get_dialog_style();
    REQUIRE(shared_style != nullptr);

    // Get expected color from the shared style
    lv_style_value_t expected_value;
    lv_style_res_t res = lv_style_get_prop(shared_style, LV_STYLE_BG_COLOR, &expected_value);
    REQUIRE(res == LV_STYLE_RES_FOUND);

    // Get actual color from ui_dialog
    lv_color_t actual = lv_obj_get_style_bg_color(dialog, LV_PART_MAIN);

    // Log colors for debugging
    uint32_t expected_rgb = lv_color_to_u32(expected_value.color) & 0x00FFFFFF;
    uint32_t actual_rgb = lv_color_to_u32(actual) & 0x00FFFFFF;
    INFO("Expected (from shared dialog_style): 0x" << std::hex << expected_rgb);
    INFO("Actual (from ui_dialog): 0x" << std::hex << actual_rgb);

    // Both should be the same card_alt color
    // Note: This may pass since both read from theme_manager_get_color("card_alt")
    // at initialization time. The real test is whether it updates on theme change.
    REQUIRE(lv_color_eq(actual, expected_value.color));

    lv_obj_delete(dialog);
}

TEST_CASE_METHOD(LVGLUITestFixture, "ui_dialog: multiple dialogs update together on theme change",
                 "[reactive-dialog]") {
    // Create multiple ui_dialog widgets
    lv_obj_t* dialog1 = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_dialog", nullptr));
    lv_obj_t* dialog2 = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_dialog", nullptr));
    lv_obj_t* dialog3 = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "ui_dialog", nullptr));

    REQUIRE(dialog1 != nullptr);
    REQUIRE(dialog2 != nullptr);
    REQUIRE(dialog3 != nullptr);

    // Get initial colors
    lv_color_t before1 = lv_obj_get_style_bg_color(dialog1, LV_PART_MAIN);
    lv_color_t before2 = lv_obj_get_style_bg_color(dialog2, LV_PART_MAIN);
    lv_color_t before3 = lv_obj_get_style_bg_color(dialog3, LV_PART_MAIN);

    // All dialogs should have the same initial color
    REQUIRE(lv_color_eq(before1, before2));
    REQUIRE(lv_color_eq(before2, before3));

    // Update to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    lv_obj_report_style_change(nullptr);

    // Get colors after theme change
    lv_color_t after1 = lv_obj_get_style_bg_color(dialog1, LV_PART_MAIN);
    lv_color_t after2 = lv_obj_get_style_bg_color(dialog2, LV_PART_MAIN);
    lv_color_t after3 = lv_obj_get_style_bg_color(dialog3, LV_PART_MAIN);

    // All dialogs should still have the same color (consistency)
    REQUIRE(lv_color_eq(after1, after2));
    REQUIRE(lv_color_eq(after2, after3));

    // And the color should have changed from before (reactivity)
    // This will FAIL with current inline style implementation
    REQUIRE_FALSE(lv_color_eq(before1, after1));

    lv_obj_delete(dialog1);
    lv_obj_delete(dialog2);
    lv_obj_delete(dialog3);
}

// ============================================================================
// ui_text Widget Reactive Style Tests - Phase 1.4
// ============================================================================
// These tests verify that text_body and text_heading widgets update their
// text color when the theme changes. By using the shared text styles from
// theme_core, text widgets become reactive to theme changes.
// ============================================================================

TEST_CASE_METHOD(LVGLUITestFixture, "text_body: text color updates on theme change",
                 "[reactive-text]") {
    // Create text_body widget via XML
    lv_obj_t* label = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "text_body", nullptr));
    REQUIRE(label != nullptr);

    // Get initial text color
    lv_color_t before = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Log initial color for debugging
    uint32_t before_rgb = lv_color_to_u32(before) & 0x00FFFFFF;
    INFO("Initial text_body text_color: 0x" << std::hex << before_rgb);

    // Update theme colors to dark mode (significantly different colors)
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0); // Light text for dark mode
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, // is_dark
                             dark_screen_bg, dark_card_bg, dark_surface, dark_text, dark_text_muted,
                             dark_text_subtle, dark_focus, dark_primary, dark_border);

    // Force LVGL style refresh cascade
    lv_obj_report_style_change(nullptr);

    // Get updated text color
    lv_color_t after = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Log updated color for debugging
    uint32_t after_rgb = lv_color_to_u32(after) & 0x00FFFFFF;
    INFO("After theme change text_color: 0x" << std::hex << after_rgb);

    // Text color should change (light mode dark text -> dark mode light text)
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(label);
}

TEST_CASE_METHOD(LVGLUITestFixture, "text_heading: text color updates on theme change",
                 "[reactive-text]") {
    // Create text_heading widget via XML (uses text_muted color)
    lv_obj_t* label = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "text_heading", nullptr));
    REQUIRE(label != nullptr);

    // Get initial text color
    lv_color_t before = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Log initial color for debugging
    uint32_t before_rgb = lv_color_to_u32(before) & 0x00FFFFFF;
    INFO("Initial text_heading text_color: 0x" << std::hex << before_rgb);

    // Update theme colors to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0); // Muted text for dark mode
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, // is_dark
                             dark_screen_bg, dark_card_bg, dark_surface, dark_text, dark_text_muted,
                             dark_text_subtle, dark_focus, dark_primary, dark_border);

    // Force LVGL style refresh cascade
    lv_obj_report_style_change(nullptr);

    // Get updated text color
    lv_color_t after = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Log updated color for debugging
    uint32_t after_rgb = lv_color_to_u32(after) & 0x00FFFFFF;
    INFO("After theme change text_color: 0x" << std::hex << after_rgb);

    // Text color should change (muted color differs between light/dark mode)
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(label);
}

TEST_CASE_METHOD(LVGLUITestFixture, "text_small: text color updates on theme change",
                 "[reactive-text]") {
    // Create text_small widget via XML (uses text_muted color)
    lv_obj_t* label = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "text_small", nullptr));
    REQUIRE(label != nullptr);

    // Get initial text color
    lv_color_t before = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Update theme colors to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    lv_obj_report_style_change(nullptr);

    // Get updated text color
    lv_color_t after = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Text color should change
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(label);
}

TEST_CASE_METHOD(LVGLUITestFixture, "text_xs: text color updates on theme change",
                 "[reactive-text]") {
    // Create text_xs widget via XML (uses text_muted color)
    lv_obj_t* label = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "text_xs", nullptr));
    REQUIRE(label != nullptr);

    // Get initial text color
    lv_color_t before = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Update theme colors to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    lv_obj_report_style_change(nullptr);

    // Get updated text color
    lv_color_t after = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Text color should change
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(label);
}

TEST_CASE_METHOD(LVGLUITestFixture, "text_button: text color updates on theme change",
                 "[reactive-text]") {
    // Create text_button widget via XML (uses text primary color)
    lv_obj_t* label = static_cast<lv_obj_t*>(lv_xml_create(test_screen(), "text_button", nullptr));
    REQUIRE(label != nullptr);

    // Get initial text color
    lv_color_t before = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Update theme colors to dark mode
    lv_color_t dark_screen_bg = lv_color_hex(0x121212);
    lv_color_t dark_card_bg = lv_color_hex(0x1E1E1E);
    lv_color_t dark_surface = lv_color_hex(0x2D2D2D);
    lv_color_t dark_text = lv_color_hex(0xE0E0E0);
    lv_color_t dark_text_muted = lv_color_hex(0xA0A0A0);
    lv_color_t dark_text_subtle = lv_color_hex(0x808080);
    lv_color_t dark_focus = lv_color_hex(0x4FC3F7);
    lv_color_t dark_primary = lv_color_hex(0x2196F3);
    lv_color_t dark_border = lv_color_hex(0x424242);

    theme_core_update_colors(true, dark_screen_bg, dark_card_bg, dark_surface, dark_text,
                             dark_text_muted, dark_text_subtle, dark_focus, dark_primary,
                             dark_border);

    lv_obj_report_style_change(nullptr);

    // Get updated text color
    lv_color_t after = lv_obj_get_style_text_color(label, LV_PART_MAIN);

    // Text color should change
    REQUIRE_FALSE(lv_color_eq(before, after));

    lv_obj_delete(label);
}
