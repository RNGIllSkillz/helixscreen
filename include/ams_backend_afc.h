// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

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

#pragma once

#include "ams_backend.h"
#include "moonraker_client.h"

#include <atomic>
#include <mutex>
#include <unordered_map>

// Forward declaration
class MoonrakerAPI;

/**
 * @file ams_backend_afc.h
 * @brief AFC-Klipper-Add-On backend implementation
 *
 * Implements the AmsBackend interface for AFC (Armored Turtle / Box Turtle)
 * multi-filament systems. Communicates with Moonraker to control AFC via
 * G-code commands and receives state updates via printer.afc.* subscriptions
 * and database lane_data queries.
 *
 * AFC Terminology Differences from Happy Hare:
 * - "Lanes" instead of "Gates"
 * - "Units" are typically called "Box Turtles" or "AFC units"
 * - Lane names may be configurable (lane1, lane2... or custom names)
 *
 * AFC State Sources:
 * - Printer object: printer.afc with status info
 * - Moonraker database: lane_data (via server.database.get_item)
 *
 * Lane Data Structure (from database):
 * {
 *   "lane1": {"color": "FF0000", "material": "PLA", "loaded": false},
 *   "lane2": {"color": "00FF00", "material": "PETG", "loaded": true}
 * }
 *
 * G-code Commands:
 * - AFC_LOAD LANE={name}   - Load filament from specified lane
 * - AFC_UNLOAD             - Unload current filament
 * - AFC_CUT LANE={name}    - Cut filament (if cutter supported)
 * - AFC_HOME               - Home the AFC system
 * - T{n}                   - Tool change (unload + load)
 */
class AmsBackendAfc : public AmsBackend {
  public:
    /**
     * @brief Construct AFC backend
     *
     * @param api Pointer to MoonrakerAPI (for sending G-code commands)
     * @param client Pointer to MoonrakerClient (for subscribing to updates)
     *
     * @note Both pointers must remain valid for the lifetime of this backend.
     */
    AmsBackendAfc(MoonrakerAPI* api, MoonrakerClient* client);

    ~AmsBackendAfc() override;

    // Lifecycle
    AmsError start() override;
    void stop() override;
    [[nodiscard]] bool is_running() const override;

    // Events
    void set_event_callback(EventCallback callback) override;

    // State queries
    [[nodiscard]] AmsSystemInfo get_system_info() const override;
    [[nodiscard]] AmsType get_type() const override;
    [[nodiscard]] GateInfo get_gate_info(int global_index) const override;
    [[nodiscard]] AmsAction get_current_action() const override;
    [[nodiscard]] int get_current_tool() const override;
    [[nodiscard]] int get_current_gate() const override;
    [[nodiscard]] bool is_filament_loaded() const override;

    // Operations
    AmsError load_filament(int gate_index) override;
    AmsError unload_filament() override;
    AmsError select_gate(int gate_index) override;
    AmsError change_tool(int tool_number) override;

    // Recovery
    AmsError recover() override;
    AmsError home() override;
    AmsError cancel() override;

    // Configuration
    AmsError set_gate_info(int gate_index, const GateInfo& info) override;
    AmsError set_tool_mapping(int tool_number, int gate_index) override;

  private:
    /**
     * @brief Handle status update notifications from Moonraker
     *
     * Called when printer.afc.* values change via notify_status_update.
     * Parses the JSON and updates internal state.
     *
     * @param notification JSON notification from Moonraker
     */
    void handle_status_update(const nlohmann::json& notification);

    /**
     * @brief Parse AFC state from Moonraker JSON
     *
     * Extracts afc object from notification and updates system_info_.
     *
     * @param afc_data JSON object containing printer.afc data
     */
    void parse_afc_state(const nlohmann::json& afc_data);

    /**
     * @brief Query lane data from Moonraker database
     *
     * AFC stores lane configuration in Moonraker's database under the
     * "AFC" namespace with key "lane_data".
     */
    void query_lane_data();

    /**
     * @brief Parse lane data from database response
     *
     * Processes the lane_data JSON object and updates system_info_.gates.
     *
     * @param lane_data JSON object containing lane configurations
     */
    void parse_lane_data(const nlohmann::json& lane_data);

    /**
     * @brief Initialize lane structures based on discovered lanes
     *
     * Called when we first receive lane data to create the correct
     * number of GateInfo entries.
     *
     * @param lane_names Vector of lane name strings
     */
    void initialize_lanes(const std::vector<std::string>& lane_names);

    /**
     * @brief Get lane name for a gate index
     *
     * AFC uses lane names (e.g., "lane1", "lane2") instead of numeric indices.
     *
     * @param gate_index Gate/lane index (0-based)
     * @return Lane name or empty string if invalid
     */
    std::string get_lane_name(int gate_index) const;

    /**
     * @brief Emit event to registered callback
     * @param event Event name
     * @param data Event data (JSON or empty)
     */
    void emit_event(const std::string& event, const std::string& data = "");

    /**
     * @brief Execute a G-code command via MoonrakerAPI
     *
     * @param gcode The G-code command to execute
     * @return AmsError indicating success or failure to queue command
     */
    AmsError execute_gcode(const std::string& gcode);

    /**
     * @brief Check common preconditions before operations
     *
     * Validates:
     * - Backend is running
     * - System is not busy
     *
     * @return AmsError (SUCCESS if ok, appropriate error otherwise)
     */
    AmsError check_preconditions() const;

    /**
     * @brief Validate gate index is within range
     *
     * @param gate_index Gate index to validate
     * @return AmsError (SUCCESS if valid, INVALID_GATE otherwise)
     */
    AmsError validate_gate_index(int gate_index) const;

    // Dependencies
    MoonrakerAPI* api_;       ///< For sending G-code commands
    MoonrakerClient* client_; ///< For subscribing to updates

    // State
    mutable std::mutex mutex_;          ///< Protects state access
    std::atomic<bool> running_{false};  ///< Backend running state
    EventCallback event_callback_;      ///< Registered event handler
    SubscriptionId subscription_id_{0}; ///< Moonraker subscription ID

    // Cached AFC state
    AmsSystemInfo system_info_;           ///< Current system state
    bool lanes_initialized_{false};       ///< Have we received lane data yet?
    std::vector<std::string> lane_names_; ///< Ordered list of lane names

    // Lane name to gate index mapping
    std::unordered_map<std::string, int> lane_name_to_index_;
};
