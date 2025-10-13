#pragma once

#include "lvgl/lvgl.h"

/**
 * @brief Controls Panel - Launcher and sub-screen management
 *
 * The controls panel provides a card-based launcher menu for accessing
 * different manual printer control screens (motion, temperature, extrusion).
 */

/**
 * @brief Initialize controls panel subjects
 *
 * Must be called BEFORE creating XML components.
 */
void ui_panel_controls_init_subjects();

/**
 * @brief Setup event handlers for controls panel launcher cards
 *
 * Wires click handlers to each launcher card.
 *
 * @param panel_obj The controls panel object returned from lv_xml_create()
 */
void ui_panel_controls_wire_events(lv_obj_t* panel_obj);

/**
 * @brief Get the controls panel object
 *
 * @return The controls panel object, or NULL if not created yet
 */
lv_obj_t* ui_panel_controls_get();

/**
 * @brief Set the controls panel object
 *
 * Called after XML creation to store reference.
 *
 * @param panel_obj The controls panel object
 */
void ui_panel_controls_set(lv_obj_t* panel_obj);
