#pragma once

#include "lvgl/lvgl.h"

/**
 * View mode for print select panel
 */
enum class PrintSelectViewMode {
    CARD = 0,  // Card grid view (default)
    LIST = 1   // List view with columns
};

/**
 * Sort column for list view
 */
enum class PrintSelectSortColumn {
    FILENAME,
    SIZE,
    MODIFIED,
    PRINT_TIME,
    FILAMENT
};

/**
 * Sort direction
 */
enum class PrintSelectSortDirection {
    ASCENDING,
    DESCENDING
};

/**
 * Initialize print select panel subjects
 * Call this BEFORE creating XML components
 */
void ui_panel_print_select_init_subjects();

/**
 * Setup print select panel after XML creation
 * Wires up event handlers and creates detail view
 * @param panel_root The root print select panel widget
 */
void ui_panel_print_select_setup(lv_obj_t* panel_root);

/**
 * Populate print select panel with test print files
 * Automatically handles both card and list views
 * @param panel_root The root print select panel widget
 */
void ui_panel_print_select_populate_test_data(lv_obj_t* panel_root);

/**
 * Toggle between card and list view
 */
void ui_panel_print_select_toggle_view();

/**
 * Sort files by specified column
 * @param column Column to sort by
 */
void ui_panel_print_select_sort_by(PrintSelectSortColumn column);

/**
 * Show delete confirmation dialog for the selected file
 */
void ui_panel_print_select_show_delete_confirmation();

/**
 * Set the selected file data (updates reactive subjects)
 * @param filename The name of the file
 * @param thumbnail_src Path to thumbnail image
 * @param print_time Formatted print time string
 * @param filament_weight Formatted filament weight string
 */
void ui_panel_print_select_set_file(const char* filename, const char* thumbnail_src,
                                    const char* print_time, const char* filament_weight);

/**
 * Show the detail view overlay
 */
void ui_panel_print_select_show_detail_view();

/**
 * Hide the detail view overlay
 */
void ui_panel_print_select_hide_detail_view();
