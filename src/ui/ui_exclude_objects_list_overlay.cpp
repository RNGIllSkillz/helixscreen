// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui_exclude_objects_list_overlay.h"

#include "ui_nav.h"
#include "ui_nav_manager.h"
#include "ui_print_exclude_object_manager.h"

#include "observer_factory.h"
#include "printer_state.h"
#include "static_panel_registry.h"
#include "theme_manager.h"

#include <spdlog/spdlog.h>

#include <cstring>
#include <memory>

namespace helix::ui {

// ============================================================================
// SINGLETON ACCESSOR
// ============================================================================

static std::unique_ptr<ExcludeObjectsListOverlay> g_exclude_objects_list_overlay;

ExcludeObjectsListOverlay& get_exclude_objects_list_overlay() {
    if (!g_exclude_objects_list_overlay) {
        g_exclude_objects_list_overlay = std::make_unique<ExcludeObjectsListOverlay>();
        StaticPanelRegistry::instance().register_destroy(
            "ExcludeObjectsListOverlay", []() { g_exclude_objects_list_overlay.reset(); });
    }
    return *g_exclude_objects_list_overlay;
}

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

ExcludeObjectsListOverlay::ExcludeObjectsListOverlay() {
    spdlog::debug("[{}] Created", get_name());
}

ExcludeObjectsListOverlay::~ExcludeObjectsListOverlay() {
    spdlog::trace("[{}] Destroyed", get_name());
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void ExcludeObjectsListOverlay::init_subjects() {
    // No local subjects needed - we observe PrinterState subjects
    subjects_initialized_ = true;
}

void ExcludeObjectsListOverlay::register_callbacks() {
    // No XML event callbacks - rows use lv_obj_add_event_cb (dynamic creation exception)
    spdlog::debug("[{}] Callbacks registered (none needed)", get_name());
}

// ============================================================================
// UI CREATION
// ============================================================================

lv_obj_t* ExcludeObjectsListOverlay::create(lv_obj_t* parent) {
    if (overlay_root_) {
        spdlog::warn("[{}] create() called but overlay already exists", get_name());
        return overlay_root_;
    }

    spdlog::debug("[{}] Creating overlay...", get_name());

    // Use base class helper for standard overlay setup (header, content padding, hidden)
    if (!create_overlay_from_xml(parent, "exclude_objects_list_overlay")) {
        spdlog::error("[{}] Failed to create overlay from XML", get_name());
        return nullptr;
    }

    // Find the dynamic list container
    objects_list_ = lv_obj_find_by_name(overlay_root_, "objects_list");
    if (!objects_list_) {
        spdlog::error("[{}] Could not find objects_list container", get_name());
    }

    spdlog::info("[{}] Overlay created", get_name());
    return overlay_root_;
}

void ExcludeObjectsListOverlay::show(lv_obj_t* parent_screen, MoonrakerAPI* api,
                                     PrinterState& printer_state,
                                     PrintExcludeObjectManager* manager) {
    spdlog::debug("[{}] show() called", get_name());

    api_ = api;
    printer_state_ = &printer_state;
    manager_ = manager;

    // Lazy create
    if (!overlay_root_ && parent_screen) {
        if (!are_subjects_initialized()) {
            init_subjects();
        }
        register_callbacks();
        create(parent_screen);
    }

    if (!overlay_root_) {
        spdlog::error("[{}] Cannot show - overlay not created", get_name());
        return;
    }

    // Register with NavigationManager for lifecycle callbacks
    NavigationManager::instance().register_overlay_instance(overlay_root_, this);

    // Push onto navigation stack (on_activate will populate the list)
    ui_nav_push_overlay(overlay_root_);
}

// ============================================================================
// LIFECYCLE HOOKS
// ============================================================================

void ExcludeObjectsListOverlay::on_activate() {
    OverlayBase::on_activate();

    if (!printer_state_)
        return;

    // Observe excluded objects changes - repopulate on change
    excluded_observer_ = ObserverGuard(
        printer_state_->get_excluded_objects_version_subject(),
        [](lv_observer_t* obs, lv_subject_t*) {
            auto* self = static_cast<ExcludeObjectsListOverlay*>(lv_observer_get_user_data(obs));
            if (self && self->is_visible()) {
                self->populate_list();
            }
        },
        this);

    // Observe defined objects changes - repopulate on change
    defined_observer_ = ObserverGuard(
        printer_state_->get_defined_objects_version_subject(),
        [](lv_observer_t* obs, lv_subject_t*) {
            auto* self = static_cast<ExcludeObjectsListOverlay*>(lv_observer_get_user_data(obs));
            if (self && self->is_visible()) {
                self->populate_list();
            }
        },
        this);

    // Repopulate to get fresh data
    populate_list();
}

void ExcludeObjectsListOverlay::on_deactivate() {
    OverlayBase::on_deactivate();

    // Release observers when not visible
    excluded_observer_.reset();
    defined_observer_.reset();
}

// ============================================================================
// LIST POPULATION
// ============================================================================

void ExcludeObjectsListOverlay::populate_list() {
    if (!objects_list_ || !printer_state_) {
        return;
    }

    // Clear existing rows
    lv_obj_clean(objects_list_);

    const auto& defined = printer_state_->get_defined_objects();
    const auto& excluded = printer_state_->get_excluded_objects();
    const auto& current = printer_state_->get_current_object();

    spdlog::debug("[{}] Populating list: {} defined, {} excluded, current='{}'", get_name(),
                  defined.size(), excluded.size(), current);

    for (const auto& name : defined) {
        bool is_excluded = excluded.count(name) > 0;
        bool is_current = (name == current);
        create_object_row(objects_list_, name, is_excluded, is_current);
    }
}

lv_obj_t* ExcludeObjectsListOverlay::create_object_row(lv_obj_t* parent, const std::string& name,
                                                       bool is_excluded, bool is_current) {
    // Row container
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(row, theme_manager_get_spacing("space_md"), 0);
    lv_obj_set_style_pad_gap(row, theme_manager_get_spacing("space_md"), 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_set_style_bg_color(row, theme_manager_get_color("card_bg"), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    // Status indicator dot (12x12 circle)
    lv_obj_t* dot = lv_obj_create(row);
    lv_obj_set_size(dot, 12, 12);
    lv_obj_set_style_radius(dot, 6, 0); // circle
    lv_obj_remove_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(dot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(dot, LV_OBJ_FLAG_EVENT_BUBBLE);

    if (is_excluded) {
        lv_obj_set_style_bg_color(dot, theme_manager_get_color("danger"), 0);
    } else if (is_current) {
        lv_obj_set_style_bg_color(dot, theme_manager_get_color("success"), 0);
    } else {
        lv_obj_set_style_bg_color(dot, theme_manager_get_color("text_muted"), 0);
    }
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);

    // Object name label
    lv_obj_t* label = lv_label_create(row);
    lv_label_set_text(label, name.c_str());
    lv_obj_set_flex_grow(label, 1);
    lv_obj_set_style_text_font(label, theme_manager_get_font("font_body"), 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Status text (right side)
    lv_obj_t* status_label = lv_label_create(row);
    lv_obj_set_style_text_font(status_label, theme_manager_get_font("font_small"), 0);
    lv_obj_set_style_text_color(status_label, theme_manager_get_color("text_muted"), 0);
    lv_obj_add_flag(status_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    if (is_excluded) {
        lv_label_set_text(status_label, "Excluded");
        lv_obj_set_style_text_color(label, theme_manager_get_color("text_muted"), 0);
        lv_obj_set_style_opa(row, 150, 0); // Reduced opacity for excluded
    } else if (is_current) {
        lv_label_set_text(status_label, "Printing");
        lv_obj_set_style_text_color(status_label, theme_manager_get_color("success"), 0);
    } else {
        lv_label_set_text(status_label, "");
    }

    // Click handler for non-excluded objects
    if (!is_excluded && manager_) {
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);

        // Allocate name string for callback
        char* name_copy = static_cast<char*>(lv_malloc(name.size() + 1));
        if (name_copy) {
            strcpy(name_copy, name.c_str());
            lv_obj_set_user_data(row, name_copy);

            // Click handler - uses singleton accessor to avoid capturing 'this'
            lv_obj_add_event_cb(
                row,
                [](lv_event_t* e) {
                    lv_obj_t* target = lv_event_get_target_obj(e);
                    char* obj_name = static_cast<char*>(lv_obj_get_user_data(target));
                    if (obj_name) {
                        auto& overlay = get_exclude_objects_list_overlay();
                        if (overlay.manager_) {
                            spdlog::info("[Exclude Objects List] Row clicked: '{}'", obj_name);
                            overlay.manager_->request_exclude(std::string(obj_name));
                        }
                    }
                },
                LV_EVENT_CLICKED, nullptr);

            // Cleanup handler to free allocated name on widget deletion
            lv_obj_add_event_cb(
                row,
                [](lv_event_t* e) {
                    lv_obj_t* obj = lv_event_get_target_obj(e);
                    char* data = static_cast<char*>(lv_obj_get_user_data(obj));
                    if (data) {
                        lv_free(data);
                        lv_obj_set_user_data(obj, nullptr);
                    }
                },
                LV_EVENT_DELETE, nullptr);
        }

        // Press feedback style
        lv_obj_set_style_bg_color(row, theme_manager_get_color("primary"), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(row, 40, LV_STATE_PRESSED);
    }

    return row;
}

} // namespace helix::ui
