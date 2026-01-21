// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui_theme_editor_overlay.h"

#include "ui_global_panel_helper.h"
#include "ui_nav.h"
#include "ui_theme.h"

#include "lvgl/src/xml/lv_xml.h"

#include <spdlog/spdlog.h>

// ============================================================================
// GLOBAL INSTANCE
// ============================================================================

DEFINE_GLOBAL_OVERLAY_STORAGE(ThemeEditorOverlay, g_theme_editor_overlay, get_theme_editor_overlay)

void init_theme_editor_overlay() {
    INIT_GLOBAL_OVERLAY(ThemeEditorOverlay, g_theme_editor_overlay);
}

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

ThemeEditorOverlay::ThemeEditorOverlay() {
    spdlog::debug("[{}] Constructor", get_name());
}

ThemeEditorOverlay::~ThemeEditorOverlay() {
    if (!lv_is_initialized()) {
        spdlog::debug("[ThemeEditorOverlay] Destroyed (LVGL already deinit)");
        return;
    }

    spdlog::debug("[ThemeEditorOverlay] Destroyed");
}

// ============================================================================
// OVERLAYBASE IMPLEMENTATION
// ============================================================================

void ThemeEditorOverlay::init_subjects() {
    if (subjects_initialized_) {
        spdlog::warn("[{}] init_subjects() called twice - ignoring", get_name());
        return;
    }

    // No local subjects needed for initial implementation

    subjects_initialized_ = true;
    spdlog::debug("[{}] Subjects initialized", get_name());
}

lv_obj_t* ThemeEditorOverlay::create(lv_obj_t* parent) {
    // Create overlay root from XML
    overlay_root_ = static_cast<lv_obj_t*>(lv_xml_create(parent, "theme_editor_overlay", nullptr));
    if (!overlay_root_) {
        spdlog::error("[{}] Failed to create overlay from XML", get_name());
        return nullptr;
    }

    // Find panel widget (content container)
    panel_ = lv_obj_find_by_name(overlay_root_, "overlay_content");
    if (!panel_) {
        spdlog::warn("[{}] Could not find overlay_content widget", get_name());
    }

    // Find swatch widgets (swatch_0 through swatch_15)
    for (size_t i = 0; i < swatch_objects_.size(); ++i) {
        char swatch_name[16];
        std::snprintf(swatch_name, sizeof(swatch_name), "swatch_%zu", i);
        swatch_objects_[i] = lv_obj_find_by_name(overlay_root_, swatch_name);
        if (!swatch_objects_[i]) {
            spdlog::trace("[{}] Swatch '{}' not found (may be added later)", get_name(),
                          swatch_name);
        }
    }

    spdlog::debug("[{}] Created overlay", get_name());
    return overlay_root_;
}

void ThemeEditorOverlay::register_callbacks() {
    // Callbacks will be registered in subsequent tasks (6.2-6.6)
    // Back button is handled by overlay_panel base component
    spdlog::debug("[{}] Callbacks registered", get_name());
}

void ThemeEditorOverlay::on_activate() {
    OverlayBase::on_activate();
    spdlog::debug("[{}] Activated", get_name());
}

void ThemeEditorOverlay::on_deactivate() {
    OverlayBase::on_deactivate();
    spdlog::debug("[{}] Deactivated", get_name());
}

void ThemeEditorOverlay::cleanup() {
    spdlog::debug("[{}] Cleanup", get_name());

    // Clear swatch references (widgets will be destroyed by LVGL)
    swatch_objects_.fill(nullptr);
    panel_ = nullptr;

    OverlayBase::cleanup();
}

// ============================================================================
// THEME EDITOR API
// ============================================================================

void ThemeEditorOverlay::load_theme(const std::string& filename) {
    std::string themes_dir = helix::get_themes_directory();
    std::string filepath = themes_dir + "/" + filename + ".json";

    helix::ThemeData loaded = helix::load_theme_from_file(filepath);
    if (!loaded.is_valid()) {
        spdlog::error("[{}] Failed to load theme from '{}'", get_name(), filepath);
        return;
    }

    // Store both copies - editing and original for revert
    editing_theme_ = loaded;
    original_theme_ = loaded;

    // Clear dirty state since we just loaded
    clear_dirty();

    // Update visual swatches
    update_swatch_colors();

    // Update property sliders
    update_property_sliders();

    spdlog::info("[{}] Loaded theme '{}' for editing", get_name(), loaded.name);
}

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

void ThemeEditorOverlay::setup_callbacks() {
    // Will be implemented in subsequent tasks
}

void ThemeEditorOverlay::update_swatch_colors() {
    for (size_t i = 0; i < swatch_objects_.size(); ++i) {
        if (!swatch_objects_[i]) {
            continue;
        }

        // Get color from editing theme
        const std::string& color_hex = editing_theme_.colors.at(i);
        if (color_hex.empty()) {
            continue;
        }

        // Parse hex color and apply to swatch background
        lv_color_t color = ui_theme_parse_hex_color(color_hex.c_str());
        lv_obj_set_style_bg_color(swatch_objects_[i], color, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(swatch_objects_[i], LV_OPA_COVER, LV_PART_MAIN);

        spdlog::trace("[{}] Set swatch {} to {}", get_name(), i, color_hex);
    }
}

void ThemeEditorOverlay::update_property_sliders() {
    // Will be implemented in subsequent tasks
    // For now, just log that we would update sliders
    spdlog::trace("[{}] Would update property sliders: border_radius={}, border_width={}, "
                  "border_opacity={}, shadow_intensity={}",
                  get_name(), editing_theme_.properties.border_radius,
                  editing_theme_.properties.border_width, editing_theme_.properties.border_opacity,
                  editing_theme_.properties.shadow_intensity);
}

void ThemeEditorOverlay::mark_dirty() {
    if (!dirty_) {
        dirty_ = true;
        spdlog::debug("[{}] Theme marked as dirty (unsaved changes)", get_name());
    }
}

void ThemeEditorOverlay::clear_dirty() {
    dirty_ = false;
    spdlog::trace("[{}] Dirty state cleared", get_name());
}

// ============================================================================
// CALLBACK STUBS (to be implemented in tasks 6.2-6.6)
// ============================================================================

void ThemeEditorOverlay::on_swatch_clicked(lv_event_t* /* e */) {
    // Will be implemented in task 6.3
}

void ThemeEditorOverlay::on_slider_changed(lv_event_t* /* e */) {
    // Will be implemented in task 6.4
}

void ThemeEditorOverlay::on_save_clicked(lv_event_t* /* e */) {
    // Will be implemented in task 6.5
}

void ThemeEditorOverlay::on_save_as_clicked(lv_event_t* /* e */) {
    // Will be implemented in task 6.5
}

void ThemeEditorOverlay::on_revert_clicked(lv_event_t* /* e */) {
    // Will be implemented in task 6.5
}

void ThemeEditorOverlay::on_close_requested(lv_event_t* /* e */) {
    // Will be implemented in task 6.6
}

void ThemeEditorOverlay::handle_swatch_click(int /* palette_index */) {
    // Will be implemented in task 6.3
}

void ThemeEditorOverlay::handle_slider_change(const char* /* slider_name */, int /* value */) {
    // Will be implemented in task 6.4
}

void ThemeEditorOverlay::show_color_picker(int /* palette_index */) {
    // Will be implemented in task 6.3
}

void ThemeEditorOverlay::show_save_as_dialog() {
    // Will be implemented in task 6.5
}

void ThemeEditorOverlay::show_discard_confirmation(std::function<void()> /* on_discard */) {
    // Will be implemented in task 6.6
}
