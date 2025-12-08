// Copyright 2025 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui_panel_base.h"

#include "print_history_data.h"

#include <vector>

/**
 * @file ui_panel_history_list.h
 * @brief Print History List Panel - Scrollable list of print jobs
 *
 * The History List Panel displays a scrollable list of all print history jobs
 * with metadata (filename, date, duration, filament type, status).
 *
 * ## Navigation:
 * - Entry: History Dashboard → "View Full History" button
 * - Back: Returns to History Dashboard
 * - Row click: Opens Detail Overlay (Stage 5 - not yet implemented)
 *
 * ## Data Flow:
 * 1. On activate, receives job list from HistoryDashboardPanel
 * 2. Dynamically creates row widgets for each job
 * 3. Caches job data for row click handling
 *
 * @see print_history_data.h for PrintHistoryJob struct
 * @see PanelBase for base class documentation
 */
class HistoryListPanel : public PanelBase {
  public:
    /**
     * @brief Construct HistoryListPanel with injected dependencies
     *
     * @param printer_state Reference to PrinterState
     * @param api Pointer to MoonrakerAPI
     */
    HistoryListPanel(PrinterState& printer_state, MoonrakerAPI* api);

    ~HistoryListPanel() override = default;

    //
    // === PanelBase Implementation ===
    //

    /**
     * @brief Initialize subjects for reactive bindings
     *
     * Creates:
     * - history_list_has_jobs: 0 = no history, 1 = has history (for empty state)
     */
    void init_subjects() override;

    /**
     * @brief Setup the list panel with widget references and event handlers
     *
     * @param panel Root panel object from lv_xml_create()
     * @param parent_screen Parent screen for overlay creation
     */
    void setup(lv_obj_t* panel, lv_obj_t* parent_screen) override;

    const char* get_name() const override {
        return "History List";
    }
    const char* get_xml_component_name() const override {
        return "history_list_panel";
    }

    //
    // === Lifecycle Hooks ===
    //

    /**
     * @brief Called when panel becomes visible
     *
     * If jobs haven't been set externally, fetches history from API.
     */
    void on_activate() override;

    /**
     * @brief Called when panel is hidden
     */
    void on_deactivate() override;

    //
    // === Public API ===
    //

    /**
     * @brief Set the jobs to display (called by dashboard when navigating)
     *
     * This avoids redundant API calls since the dashboard already has the data.
     *
     * @param jobs Vector of print history jobs
     */
    void set_jobs(const std::vector<PrintHistoryJob>& jobs);

    /**
     * @brief Refresh the list from the API
     */
    void refresh_from_api();

  private:
    //
    // === Widget References ===
    //

    lv_obj_t* list_content_ = nullptr; ///< Scrollable content area
    lv_obj_t* list_rows_ = nullptr;    ///< Container for row widgets
    lv_obj_t* empty_state_ = nullptr;  ///< Empty state message container

    //
    // === State ===
    //

    std::vector<PrintHistoryJob> jobs_; ///< Cached job list
    bool jobs_received_ = false;        ///< True if jobs were set externally

    //
    // === Subject for empty state binding ===
    //

    lv_subject_t subject_has_jobs_; ///< 0 = no jobs (show empty), 1 = has jobs (hide empty)

    //
    // === Internal Methods ===
    //

    /**
     * @brief Populate the list with row widgets
     *
     * Clears existing rows and creates new ones from jobs_ vector.
     */
    void populate_list();

    /**
     * @brief Clear all row widgets from the list
     */
    void clear_list();

    /**
     * @brief Update the empty state visibility based on job count
     */
    void update_empty_state();

    /**
     * @brief Get status color for a job status
     *
     * @param status Job status enum
     * @return Hex color string (e.g., "#00C853")
     */
    static const char* get_status_color(PrintJobStatus status);

    /**
     * @brief Get display text for a job status
     *
     * @param status Job status enum
     * @return Display string (e.g., "Completed", "Failed")
     */
    static const char* get_status_text(PrintJobStatus status);

    //
    // === Click Handlers ===
    //

    /**
     * @brief Attach click handler to a row widget
     *
     * @param row Row widget
     * @param index Index into jobs_ vector
     */
    void attach_row_click_handler(lv_obj_t* row, size_t index);

    /**
     * @brief Handle row click - opens detail overlay (Stage 5)
     *
     * @param index Index of clicked row
     */
    void handle_row_click(size_t index);

    // Static callback wrapper for row clicks
    static void on_row_clicked_static(lv_event_t* e);
};

/**
 * @brief Global instance accessor
 *
 * Returns reference to singleton HistoryListPanel used by main.cpp.
 */
HistoryListPanel& get_global_history_list_panel();

/**
 * @brief Initialize the global HistoryListPanel instance
 *
 * Must be called by main.cpp before accessing get_global_history_list_panel().
 *
 * @param printer_state Reference to PrinterState
 * @param api Pointer to MoonrakerAPI
 */
void init_global_history_list_panel(PrinterState& printer_state, MoonrakerAPI* api);
