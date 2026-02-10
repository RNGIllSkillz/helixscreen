// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file ui_settings_telemetry_data.cpp
 * @brief Implementation of TelemetryDataOverlay
 */

#include "ui_settings_telemetry_data.h"

#include "ui_event_safety.h"
#include "ui_nav_manager.h"
#include "ui_toast.h"

#include "lvgl/src/others/translation/lv_translation.h"
#include "settings_manager.h"
#include "static_panel_registry.h"
#include "system/telemetry_manager.h"
#include "theme_manager.h"

#include <spdlog/spdlog.h>

#include <memory>

namespace helix::settings {

// ============================================================================
// SINGLETON ACCESSOR
// ============================================================================

static std::unique_ptr<TelemetryDataOverlay> g_telemetry_data_overlay;

TelemetryDataOverlay& get_telemetry_data_overlay() {
    if (!g_telemetry_data_overlay) {
        g_telemetry_data_overlay = std::make_unique<TelemetryDataOverlay>();
        StaticPanelRegistry::instance().register_destroy(
            "TelemetryDataOverlay", []() { g_telemetry_data_overlay.reset(); });
    }
    return *g_telemetry_data_overlay;
}

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

TelemetryDataOverlay::TelemetryDataOverlay() {
    spdlog::debug("[{}] Created", get_name());
}

TelemetryDataOverlay::~TelemetryDataOverlay() {
    if (subjects_initialized_) {
        deinit_subjects_base(subjects_);
    }
    spdlog::trace("[{}] Destroyed", get_name());
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void TelemetryDataOverlay::init_subjects() {
    init_subjects_guarded([this]() {
        // Status text subject: "Telemetry Enabled" / "Telemetry Disabled"
        UI_MANAGED_SUBJECT_STRING(status_subject_, status_buf_, "Telemetry",
                                  "telemetry_data_status", subjects_);

        // Detail text subject: "N events queued"
        UI_MANAGED_SUBJECT_STRING(detail_subject_, detail_buf_, "0 events queued",
                                  "telemetry_data_detail", subjects_);

        // Count subject for show/hide empty state vs event list
        UI_MANAGED_SUBJECT_INT(count_subject_, 0, "telemetry_data_count", subjects_);
    });
}

void TelemetryDataOverlay::register_callbacks() {
    lv_xml_register_event_cb(nullptr, "on_telemetry_clear_queue", on_telemetry_clear_queue);

    spdlog::debug("[{}] Callbacks registered", get_name());
}

// ============================================================================
// UI CREATION
// ============================================================================

lv_obj_t* TelemetryDataOverlay::create(lv_obj_t* parent) {
    if (overlay_root_) {
        spdlog::warn("[{}] create() called but overlay already exists", get_name());
        return overlay_root_;
    }

    spdlog::debug("[{}] Creating overlay...", get_name());

    overlay_root_ =
        static_cast<lv_obj_t*>(lv_xml_create(parent, "telemetry_data_overlay", nullptr));
    if (!overlay_root_) {
        spdlog::error("[{}] Failed to create overlay from XML", get_name());
        return nullptr;
    }

    // Initially hidden until show() pushes it
    lv_obj_add_flag(overlay_root_, LV_OBJ_FLAG_HIDDEN);

    spdlog::info("[{}] Overlay created", get_name());
    return overlay_root_;
}

void TelemetryDataOverlay::show(lv_obj_t* parent_screen) {
    spdlog::debug("[{}] show() called", get_name());

    parent_screen_ = parent_screen;

    // Ensure subjects and callbacks are initialized
    if (!subjects_initialized_) {
        init_subjects();
        register_callbacks();
    }

    // Lazy create overlay
    if (!overlay_root_ && parent_screen_) {
        create(parent_screen_);
    }

    if (!overlay_root_) {
        spdlog::error("[{}] Cannot show - overlay not created", get_name());
        return;
    }

    // Register for lifecycle callbacks
    NavigationManager::instance().register_overlay_instance(overlay_root_, this);

    // Push onto navigation stack (on_activate will populate events)
    ui_nav_push_overlay(overlay_root_);
}

// ============================================================================
// LIFECYCLE HOOKS
// ============================================================================

void TelemetryDataOverlay::on_activate() {
    OverlayBase::on_activate();

    update_status();
    populate_events();
}

void TelemetryDataOverlay::on_deactivate() {
    OverlayBase::on_deactivate();
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

void TelemetryDataOverlay::update_status() {
    if (!subjects_initialized_) {
        return;
    }

    auto& telemetry = TelemetryManager::instance();
    bool enabled = telemetry.is_enabled();
    size_t count = telemetry.queue_size();

    // Update status text
    const char* status_text = enabled ? "Telemetry Enabled" : "Telemetry Disabled";
    lv_subject_copy_string(&status_subject_, status_text);

    // Update detail text with event count
    if (count == 0) {
        lv_subject_copy_string(&detail_subject_, "No events queued");
    } else if (count == 1) {
        lv_subject_copy_string(&detail_subject_, "1 event queued");
    } else {
        snprintf(detail_buf_, sizeof(detail_buf_), "%zu events queued", count);
        lv_subject_copy_string(&detail_subject_, detail_buf_);
    }

    // Update count subject for show/hide logic
    lv_subject_set_int(&count_subject_, static_cast<int32_t>(count));

    spdlog::debug("[{}] Status updated: {} events, enabled={}", get_name(), count, enabled);
}

void TelemetryDataOverlay::populate_events() {
    if (!overlay_root_) {
        return;
    }

    lv_obj_t* event_list = lv_obj_find_by_name(overlay_root_, "event_list");
    if (!event_list) {
        spdlog::warn("[{}] Could not find event_list widget", get_name());
        return;
    }

    // Clear existing children
    lv_obj_clean(event_list);

    auto& telemetry = TelemetryManager::instance();
    auto snapshot = telemetry.get_queue_snapshot();

    if (!snapshot.is_array() || snapshot.empty()) {
        spdlog::debug("[{}] No events to display", get_name());
        return;
    }

    for (const auto& event : snapshot) {
        // Create a card for each event
        lv_obj_t* card = lv_obj_create(event_list);
        if (!card) {
            continue;
        }

        // Style the card
        lv_obj_set_width(card, lv_pct(100));
        lv_obj_set_height(card, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(card, theme_manager_get_color("card_bg"), 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(card, 8, 0);
        lv_obj_set_style_pad_all(card, 12, 0);
        lv_obj_set_style_pad_gap(card, 4, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        // Event type (heading)
        std::string event_type = "Unknown Event";
        if (event.contains("type") && event["type"].is_string()) {
            event_type = event["type"].get<std::string>();
            // Capitalize and format type
            if (event_type == "session") {
                event_type = "Session Start";
            } else if (event_type == "print_outcome") {
                event_type = "Print Outcome";
            }
        }

        lv_obj_t* type_label = lv_label_create(card);
        lv_label_set_text(type_label, event_type.c_str());
        lv_obj_set_style_text_color(type_label, theme_manager_get_color("text"), 0);
        lv_obj_set_style_text_font(type_label, lv_font_get_default(), 0);

        // Timestamp
        if (event.contains("timestamp") && event["timestamp"].is_string()) {
            std::string ts = event["timestamp"].get<std::string>();
            lv_obj_t* ts_label = lv_label_create(card);
            lv_label_set_text(ts_label, ts.c_str());
            lv_obj_set_style_text_color(ts_label, theme_manager_get_color("text_muted"), 0);
            lv_obj_set_style_text_font(ts_label, lv_font_get_default(), 0);
        }

        // Key fields based on event type
        std::string type_str;
        if (event.contains("type") && event["type"].is_string()) {
            type_str = event["type"].get<std::string>();
        }

        if (type_str == "session") {
            // Show version and platform
            auto add_field = [&](const char* key, const char* display_name) {
                if (event.contains(key) && event[key].is_string()) {
                    std::string text =
                        std::string(display_name) + ": " + event[key].get<std::string>();
                    lv_obj_t* label = lv_label_create(card);
                    lv_label_set_text(label, text.c_str());
                    lv_obj_set_style_text_color(label, theme_manager_get_color("text_subtle"), 0);
                    lv_obj_set_style_text_font(label, lv_font_get_default(), 0);
                }
            };
            add_field("version", "Version");
            add_field("platform", "Platform");
            add_field("display", "Display");
        } else if (type_str == "print_outcome") {
            // Show outcome and key print details
            auto add_field_str = [&](const char* key, const char* display_name) {
                if (event.contains(key) && event[key].is_string()) {
                    std::string text =
                        std::string(display_name) + ": " + event[key].get<std::string>();
                    lv_obj_t* label = lv_label_create(card);
                    lv_label_set_text(label, text.c_str());
                    lv_obj_set_style_text_color(label, theme_manager_get_color("text_subtle"), 0);
                    lv_obj_set_style_text_font(label, lv_font_get_default(), 0);
                }
            };
            auto add_field_num = [&](const char* key, const char* display_name,
                                     const char* suffix) {
                if (event.contains(key) && event[key].is_number()) {
                    char buf[64];
                    if (event[key].is_number_integer()) {
                        snprintf(buf, sizeof(buf), "%s: %d%s", display_name, event[key].get<int>(),
                                 suffix);
                    } else {
                        snprintf(buf, sizeof(buf), "%s: %.1f%s", display_name,
                                 event[key].get<double>(), suffix);
                    }
                    lv_obj_t* label = lv_label_create(card);
                    lv_label_set_text(label, buf);
                    lv_obj_set_style_text_color(label, theme_manager_get_color("text_subtle"), 0);
                    lv_obj_set_style_text_font(label, lv_font_get_default(), 0);
                }
            };
            add_field_str("outcome", "Outcome");
            add_field_num("duration_sec", "Duration", "s");
            add_field_str("filament_type", "Filament");
            add_field_num("nozzle_temp", "Nozzle",
                          "\xC2\xB0"
                          "C");
            add_field_num("bed_temp", "Bed",
                          "\xC2\xB0"
                          "C");
        }

        // Show the hashed device ID (truncated for readability)
        if (event.contains("device_id") && event["device_id"].is_string()) {
            std::string device_id = event["device_id"].get<std::string>();
            if (device_id.length() > 16) {
                device_id = device_id.substr(0, 16) + "...";
            }
            std::string text = "Device: " + device_id;
            lv_obj_t* label = lv_label_create(card);
            lv_label_set_text(label, text.c_str());
            lv_obj_set_style_text_color(label, theme_manager_get_color("text_subtle"), 0);
            lv_obj_set_style_text_font(label, lv_font_get_default(), 0);
        }
    }

    spdlog::debug("[{}] Populated {} event cards", get_name(), snapshot.size());
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void TelemetryDataOverlay::handle_clear_queue() {
    auto& telemetry = TelemetryManager::instance();
    telemetry.clear_queue();

    ui_toast_show(ToastSeverity::SUCCESS, lv_tr("Telemetry queue cleared"), 2000);
    spdlog::info("[{}] Queue cleared by user", get_name());

    // Refresh display
    update_status();
    populate_events();
}

// ============================================================================
// STATIC CALLBACKS
// ============================================================================

void TelemetryDataOverlay::on_telemetry_clear_queue(lv_event_t* /*e*/) {
    LVGL_SAFE_EVENT_CB_BEGIN("[TelemetryDataOverlay] on_telemetry_clear_queue");
    get_telemetry_data_overlay().handle_clear_queue();
    LVGL_SAFE_EVENT_CB_END();
}

} // namespace helix::settings
