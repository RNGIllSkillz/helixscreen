// Copyright 2025 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui_panel_history_list.h"

#include "ui_nav.h"
#include "ui_panel_common.h"

#include "moonraker_api.h"

#include <spdlog/spdlog.h>

#include <lvgl.h>

// Global instance (singleton pattern)
static std::unique_ptr<HistoryListPanel> g_history_list_panel;

HistoryListPanel& get_global_history_list_panel() {
    if (!g_history_list_panel) {
        spdlog::error("get_global_history_list_panel() called before initialization!");
        throw std::runtime_error("HistoryListPanel not initialized");
    }
    return *g_history_list_panel;
}

void init_global_history_list_panel(PrinterState& printer_state, MoonrakerAPI* api) {
    g_history_list_panel = std::make_unique<HistoryListPanel>(printer_state, api);
    spdlog::debug("[History List] Global instance initialized");
}

// ============================================================================
// Constructor
// ============================================================================

HistoryListPanel::HistoryListPanel(PrinterState& printer_state, MoonrakerAPI* api)
    : PanelBase(printer_state, api) {
    spdlog::debug("[{}] Constructed", get_name());
}

// ============================================================================
// PanelBase Implementation
// ============================================================================

void HistoryListPanel::init_subjects() {
    // Initialize subject for empty state binding
    lv_subject_init_int(&subject_has_jobs_, 0);
    lv_xml_register_subject(nullptr, "history_list_has_jobs", &subject_has_jobs_);

    spdlog::debug("[{}] Subjects initialized", get_name());
}

void HistoryListPanel::setup(lv_obj_t* panel, lv_obj_t* parent_screen) {
    panel_ = panel;
    parent_screen_ = parent_screen;

    // Get widget references
    list_content_ = lv_obj_find_by_name(panel_, "list_content");
    list_rows_ = lv_obj_find_by_name(panel_, "list_rows");
    empty_state_ = lv_obj_find_by_name(panel_, "empty_state");

    spdlog::debug("[{}] Widget refs - content: {}, rows: {}, empty: {}", get_name(),
                  list_content_ != nullptr, list_rows_ != nullptr, empty_state_ != nullptr);

    // Wire up back button to navigation system
    ui_panel_setup_back_button(panel_);

    spdlog::info("[{}] Setup complete", get_name());
}

// ============================================================================
// Lifecycle Hooks
// ============================================================================

void HistoryListPanel::on_activate() {
    spdlog::debug("[{}] Activated - jobs_received: {}, job_count: {}", get_name(), jobs_received_,
                  jobs_.size());

    if (!jobs_received_) {
        // Jobs weren't set by dashboard, fetch from API
        refresh_from_api();
    } else {
        // Jobs were provided, just populate the list
        populate_list();
    }
}

void HistoryListPanel::on_deactivate() {
    spdlog::debug("[{}] Deactivated", get_name());
    // Clear the received flag so next activation will refresh
    jobs_received_ = false;
}

// ============================================================================
// Public API
// ============================================================================

void HistoryListPanel::set_jobs(const std::vector<PrintHistoryJob>& jobs) {
    jobs_ = jobs;
    jobs_received_ = true;
    spdlog::debug("[{}] Jobs set: {} items", get_name(), jobs_.size());
}

void HistoryListPanel::refresh_from_api() {
    if (!api_) {
        spdlog::warn("[{}] Cannot refresh: API not set", get_name());
        return;
    }

    spdlog::debug("[{}] Fetching history from API", get_name());

    api_->get_history_list(
        200, // limit
        0,   // start
        0.0, // since (no filter)
        0.0, // before (no filter)
        [this](const std::vector<PrintHistoryJob>& jobs, uint64_t total) {
            spdlog::info("[{}] Received {} jobs (total: {})", get_name(), jobs.size(), total);
            jobs_ = jobs;
            populate_list();
        },
        [this](const MoonrakerError& error) {
            spdlog::error("[{}] Failed to fetch history: {}", get_name(), error.message);
            jobs_.clear();
            populate_list();
        });
}

// ============================================================================
// Internal Methods
// ============================================================================

void HistoryListPanel::populate_list() {
    if (!list_rows_) {
        spdlog::error("[{}] Cannot populate: list_rows container is null", get_name());
        return;
    }

    // Clear existing rows
    clear_list();

    // Update empty state
    update_empty_state();

    if (jobs_.empty()) {
        spdlog::debug("[{}] No jobs to display", get_name());
        return;
    }

    spdlog::debug("[{}] Populating list with {} jobs", get_name(), jobs_.size());

    for (size_t i = 0; i < jobs_.size(); ++i) {
        const auto& job = jobs_[i];

        // Get status info
        const char* status_color = get_status_color(job.status);
        const char* status_text = get_status_text(job.status);

        // Build attrs for row creation
        const char* attrs[] = {"filename",
                               job.filename.c_str(),
                               "date",
                               job.date_str.c_str(),
                               "duration",
                               job.duration_str.c_str(),
                               "filament_type",
                               job.filament_type.empty() ? "Unknown" : job.filament_type.c_str(),
                               "status",
                               status_text,
                               "status_color",
                               status_color,
                               NULL};

        lv_obj_t* row =
            static_cast<lv_obj_t*>(lv_xml_create(list_rows_, "history_list_row", attrs));

        if (row) {
            attach_row_click_handler(row, i);
        } else {
            spdlog::warn("[{}] Failed to create row for job {}", get_name(), i);
        }
    }

    spdlog::debug("[{}] List populated with {} rows", get_name(), jobs_.size());
}

void HistoryListPanel::clear_list() {
    if (!list_rows_)
        return;

    // Remove all children from the list container
    uint32_t child_count = lv_obj_get_child_count(list_rows_);
    for (int32_t i = child_count - 1; i >= 0; --i) {
        lv_obj_t* child = lv_obj_get_child(list_rows_, i);
        if (child) {
            lv_obj_delete(child);
        }
    }
}

void HistoryListPanel::update_empty_state() {
    int has_jobs = jobs_.empty() ? 0 : 1;
    lv_subject_set_int(&subject_has_jobs_, has_jobs);
    spdlog::debug("[{}] Empty state updated: has_jobs={}", get_name(), has_jobs);
}

const char* HistoryListPanel::get_status_color(PrintJobStatus status) {
    switch (status) {
    case PrintJobStatus::COMPLETED:
        return "#00C853"; // Green
    case PrintJobStatus::CANCELLED:
        return "#FF9800"; // Orange
    case PrintJobStatus::ERROR:
        return "#F44336"; // Red
    case PrintJobStatus::IN_PROGRESS:
        return "#2196F3"; // Blue
    default:
        return "#9E9E9E"; // Gray
    }
}

const char* HistoryListPanel::get_status_text(PrintJobStatus status) {
    switch (status) {
    case PrintJobStatus::COMPLETED:
        return "Completed";
    case PrintJobStatus::CANCELLED:
        return "Cancelled";
    case PrintJobStatus::ERROR:
        return "Failed";
    case PrintJobStatus::IN_PROGRESS:
        return "In Progress";
    default:
        return "Unknown";
    }
}

// ============================================================================
// Click Handlers
// ============================================================================

void HistoryListPanel::attach_row_click_handler(lv_obj_t* row, size_t index) {
    // Store index in user data (cast to void* for LVGL)
    lv_obj_set_user_data(row, reinterpret_cast<void*>(index));

    // Find the actual clickable row element
    lv_obj_t* history_row = lv_obj_find_by_name(row, "history_row");
    if (history_row) {
        lv_obj_set_user_data(history_row, this);
        // Store index as a property we can retrieve
        // Using the row widget's index stored in parent's user_data
        lv_obj_add_event_cb(history_row, on_row_clicked_static, LV_EVENT_CLICKED, row);
    }
}

void HistoryListPanel::on_row_clicked_static(lv_event_t* e) {
    lv_obj_t* row_container = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
    lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));

    // Get the panel instance from the clickable element
    HistoryListPanel* panel = static_cast<HistoryListPanel*>(lv_obj_get_user_data(target));
    if (!panel || !row_container)
        return;

    // Get the index from the row container
    size_t index = reinterpret_cast<size_t>(lv_obj_get_user_data(row_container));
    panel->handle_row_click(index);
}

void HistoryListPanel::handle_row_click(size_t index) {
    if (index >= jobs_.size()) {
        spdlog::warn("[{}] Invalid row index: {}", get_name(), index);
        return;
    }

    const auto& job = jobs_[index];
    spdlog::info("[{}] Row clicked: {} ({})", get_name(), job.filename,
                 get_status_text(job.status));

    // TODO: Stage 5 - Open detail overlay
    // For now, just log the click
    spdlog::debug("[{}] Detail overlay not yet implemented (Stage 5)", get_name());
}
