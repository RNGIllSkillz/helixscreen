#pragma once

#include "lvgl/lvgl.h"

/**
 * Callback function signature for keypad confirmation
 * @param value The confirmed numeric value (clamped to min/max)
 * @param user_data User data pointer passed to ui_keypad_show()
 */
typedef void (*ui_keypad_callback_t)(float value, void* user_data);

/**
 * Configuration for numeric keypad
 */
struct ui_keypad_config_t {
	float initial_value;      // Initial value to display
	float min_value;          // Minimum allowed value
	float max_value;          // Maximum allowed value
	const char* title_label;  // Title label (e.g., "Nozzle Temp", "Heat Bed Temp")
	const char* unit_label;   // Unit label (e.g., "°C", "mm")
	bool allow_decimal;       // Enable decimal point button
	bool allow_negative;      // Enable negative sign button
	ui_keypad_callback_t callback;  // Called on OK confirmation
	void* user_data;          // User data passed to callback
};

/**
 * Initialize the keypad modal component
 * Creates the modal widget and stores reference
 * Call once after component registration
 * @param parent Parent widget (usually screen or panel root)
 */
void ui_keypad_init(lv_obj_t* parent);

/**
 * Show the numeric keypad modal
 * @param config Configuration and callback
 */
void ui_keypad_show(const ui_keypad_config_t* config);

/**
 * Hide the numeric keypad modal (cancel)
 * Does NOT invoke callback
 */
void ui_keypad_hide();

/**
 * Check if keypad is currently visible
 * @return true if visible, false if hidden
 */
bool ui_keypad_is_visible();
