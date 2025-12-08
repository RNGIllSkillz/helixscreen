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

#include "ams_backend_afc.h"

#include "moonraker_api.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <sstream>

// ============================================================================
// Construction / Destruction
// ============================================================================

AmsBackendAfc::AmsBackendAfc(MoonrakerAPI* api, MoonrakerClient* client)
    : api_(api), client_(client) {
    // Initialize system info with AFC defaults
    system_info_.type = AmsType::AFC;
    system_info_.type_name = "AFC";
    system_info_.version = "unknown";
    system_info_.current_tool = -1;
    system_info_.current_gate = -1;
    system_info_.filament_loaded = false;
    system_info_.action = AmsAction::IDLE;
    system_info_.total_gates = 0;
    // AFC capabilities - may vary by configuration
    system_info_.supports_endless_spool = false;
    system_info_.supports_spoolman = true;
    system_info_.supports_tool_mapping = true;
    system_info_.supports_bypass = false; // AFC typically doesn't have bypass

    spdlog::debug("[AMS AFC] Backend created");
}

AmsBackendAfc::~AmsBackendAfc() {
    stop();
}

// ============================================================================
// Lifecycle Management
// ============================================================================

AmsError AmsBackendAfc::start() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (running_) {
        return AmsErrorHelper::success();
    }

    if (!client_) {
        spdlog::error("[AMS AFC] Cannot start: MoonrakerClient is null");
        return AmsErrorHelper::not_connected("MoonrakerClient not provided");
    }

    if (!api_) {
        spdlog::error("[AMS AFC] Cannot start: MoonrakerAPI is null");
        return AmsErrorHelper::not_connected("MoonrakerAPI not provided");
    }

    // Register for status update notifications from Moonraker
    // AFC state comes via notify_status_update when printer.afc.* changes
    subscription_id_ = client_->register_notify_update(
        [this](const nlohmann::json& notification) { handle_status_update(notification); });

    if (subscription_id_ == INVALID_SUBSCRIPTION_ID) {
        spdlog::error("[AMS AFC] Failed to register for status updates");
        return AmsErrorHelper::not_connected("Failed to subscribe to Moonraker updates");
    }

    running_ = true;
    spdlog::info("[AMS AFC] Backend started, subscription ID: {}", subscription_id_);

    // Emit initial state event (state may be empty until first Moonraker update)
    // Lane data will be populated when first status update arrives
    emit_event(EVENT_STATE_CHANGED);

    return AmsErrorHelper::success();
}

void AmsBackendAfc::stop() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!running_) {
        return;
    }

    // Unsubscribe from Moonraker updates
    if (client_ && subscription_id_ != INVALID_SUBSCRIPTION_ID) {
        client_->unsubscribe_notify_update(subscription_id_);
        subscription_id_ = INVALID_SUBSCRIPTION_ID;
    }

    running_ = false;
    spdlog::info("[AMS AFC] Backend stopped");
}

bool AmsBackendAfc::is_running() const {
    return running_;
}

// ============================================================================
// Event System
// ============================================================================

void AmsBackendAfc::set_event_callback(EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    event_callback_ = std::move(callback);
}

void AmsBackendAfc::emit_event(const std::string& event, const std::string& data) {
    EventCallback cb;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cb = event_callback_;
    }

    if (cb) {
        cb(event, data);
    }
}

// ============================================================================
// State Queries
// ============================================================================

AmsSystemInfo AmsBackendAfc::get_system_info() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return system_info_;
}

AmsType AmsBackendAfc::get_type() const {
    return AmsType::AFC;
}

GateInfo AmsBackendAfc::get_gate_info(int global_index) const {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto* gate = system_info_.get_gate_global(global_index);
    if (gate) {
        return *gate;
    }

    // Return empty gate info for invalid index
    GateInfo empty;
    empty.gate_index = -1;
    empty.global_index = -1;
    return empty;
}

AmsAction AmsBackendAfc::get_current_action() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return system_info_.action;
}

int AmsBackendAfc::get_current_tool() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return system_info_.current_tool;
}

int AmsBackendAfc::get_current_gate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return system_info_.current_gate;
}

bool AmsBackendAfc::is_filament_loaded() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return system_info_.filament_loaded;
}

// ============================================================================
// Moonraker Status Update Handling
// ============================================================================

void AmsBackendAfc::handle_status_update(const nlohmann::json& notification) {
    // notify_status_update has format: { "method": "notify_status_update", "params": [{ ... },
    // timestamp] }
    if (!notification.contains("params") || !notification["params"].is_array() ||
        notification["params"].empty()) {
        return;
    }

    const auto& params = notification["params"][0];
    if (!params.is_object()) {
        return;
    }

    // Check if this notification contains AFC data
    if (!params.contains("afc")) {
        return;
    }

    const auto& afc_data = params["afc"];
    if (!afc_data.is_object()) {
        return;
    }

    spdlog::trace("[AMS AFC] Received AFC status update");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        parse_afc_state(afc_data);
    }

    emit_event(EVENT_STATE_CHANGED);
}

void AmsBackendAfc::parse_afc_state(const nlohmann::json& afc_data) {
    // Parse current lane/gate (AFC may report this as "current_lane" or similar)
    if (afc_data.contains("current_lane") && afc_data["current_lane"].is_string()) {
        std::string lane_name = afc_data["current_lane"].get<std::string>();
        auto it = lane_name_to_index_.find(lane_name);
        if (it != lane_name_to_index_.end()) {
            system_info_.current_gate = it->second;
            spdlog::trace("[AMS AFC] Current lane: {} (gate {})", lane_name,
                          system_info_.current_gate);
        }
    }

    // Parse current tool
    if (afc_data.contains("current_tool") && afc_data["current_tool"].is_number_integer()) {
        system_info_.current_tool = afc_data["current_tool"].get<int>();
        spdlog::trace("[AMS AFC] Current tool: {}", system_info_.current_tool);
    }

    // Parse filament loaded state
    if (afc_data.contains("filament_loaded") && afc_data["filament_loaded"].is_boolean()) {
        system_info_.filament_loaded = afc_data["filament_loaded"].get<bool>();
        spdlog::trace("[AMS AFC] Filament loaded: {}", system_info_.filament_loaded);
    }

    // Parse action/status
    if (afc_data.contains("status") && afc_data["status"].is_string()) {
        std::string status_str = afc_data["status"].get<std::string>();
        system_info_.action = ams_action_from_string(status_str);
        system_info_.operation_detail = status_str;
        spdlog::trace("[AMS AFC] Status: {} ({})", ams_action_to_string(system_info_.action),
                      status_str);
    }

    // Parse lanes array if present (some AFC versions provide this)
    if (afc_data.contains("lanes") && afc_data["lanes"].is_object()) {
        parse_lane_data(afc_data["lanes"]);
    }

    // Parse unit information if available
    if (afc_data.contains("units") && afc_data["units"].is_array()) {
        // AFC may report multiple units (Box Turtles)
        // Update unit names and connection status
        const auto& units = afc_data["units"];
        for (size_t i = 0; i < units.size() && i < system_info_.units.size(); ++i) {
            if (units[i].is_object()) {
                if (units[i].contains("name") && units[i]["name"].is_string()) {
                    system_info_.units[i].name = units[i]["name"].get<std::string>();
                }
                if (units[i].contains("connected") && units[i]["connected"].is_boolean()) {
                    system_info_.units[i].connected = units[i]["connected"].get<bool>();
                }
            }
        }
    }
}

void AmsBackendAfc::query_lane_data() {
    if (!client_) {
        spdlog::warn("[AMS AFC] Cannot query lane data: client is null");
        return;
    }

    // Query Moonraker database for AFC lane_data
    // Method: server.database.get_item
    // Params: { "namespace": "AFC", "key": "lane_data" }
    nlohmann::json params = {{"namespace", "AFC"}, {"key", "lane_data"}};

    client_->send_jsonrpc(
        "server.database.get_item", params,
        [this](const nlohmann::json& response) {
            if (response.contains("value") && response["value"].is_object()) {
                std::lock_guard<std::mutex> lock(mutex_);
                parse_lane_data(response["value"]);
                emit_event(EVENT_STATE_CHANGED);
            }
        },
        [](const MoonrakerError& err) {
            spdlog::warn("[AMS AFC] Failed to query lane_data: {}", err.message);
        });
}

void AmsBackendAfc::parse_lane_data(const nlohmann::json& lane_data) {
    // Lane data format:
    // {
    //   "lane1": {"color": "FF0000", "material": "PLA", "loaded": false, ...},
    //   "lane2": {"color": "00FF00", "material": "PETG", "loaded": true, ...}
    // }

    // Extract lane names and sort them for consistent ordering
    std::vector<std::string> new_lane_names;
    for (auto it = lane_data.begin(); it != lane_data.end(); ++it) {
        new_lane_names.push_back(it.key());
    }
    std::sort(new_lane_names.begin(), new_lane_names.end());

    // Initialize lanes if this is the first time or count changed
    if (!lanes_initialized_ || new_lane_names.size() != lane_names_.size()) {
        initialize_lanes(new_lane_names);
    }

    // Update lane information
    for (size_t i = 0; i < lane_names_.size() && !system_info_.units.empty(); ++i) {
        const std::string& lane_name = lane_names_[i];
        if (!lane_data.contains(lane_name) || !lane_data[lane_name].is_object()) {
            continue;
        }

        const auto& lane = lane_data[lane_name];
        auto& gate = system_info_.units[0].gates[i];

        // Parse color (AFC uses hex string without 0x prefix)
        if (lane.contains("color") && lane["color"].is_string()) {
            std::string color_str = lane["color"].get<std::string>();
            try {
                gate.color_rgb = static_cast<uint32_t>(std::stoul(color_str, nullptr, 16));
            } catch (...) {
                gate.color_rgb = AMS_DEFAULT_GATE_COLOR;
            }
        }

        // Parse material
        if (lane.contains("material") && lane["material"].is_string()) {
            gate.material = lane["material"].get<std::string>();
        }

        // Parse loaded state
        if (lane.contains("loaded") && lane["loaded"].is_boolean()) {
            bool loaded = lane["loaded"].get<bool>();
            if (loaded) {
                gate.status = GateStatus::LOADED;
                system_info_.current_gate = static_cast<int>(i);
                system_info_.filament_loaded = true;
            } else {
                // Check if filament is available (not loaded but present)
                if (lane.contains("available") && lane["available"].is_boolean() &&
                    lane["available"].get<bool>()) {
                    gate.status = GateStatus::AVAILABLE;
                } else if (lane.contains("empty") && lane["empty"].is_boolean() &&
                           lane["empty"].get<bool>()) {
                    gate.status = GateStatus::EMPTY;
                } else {
                    // Default to available if not explicitly empty
                    gate.status = GateStatus::AVAILABLE;
                }
            }
        }

        // Parse spool information if available
        if (lane.contains("spool_id") && lane["spool_id"].is_number_integer()) {
            gate.spoolman_id = lane["spool_id"].get<int>();
        }

        if (lane.contains("brand") && lane["brand"].is_string()) {
            gate.brand = lane["brand"].get<std::string>();
        }

        if (lane.contains("remaining_weight") && lane["remaining_weight"].is_number()) {
            gate.remaining_weight_g = lane["remaining_weight"].get<float>();
        }

        if (lane.contains("total_weight") && lane["total_weight"].is_number()) {
            gate.total_weight_g = lane["total_weight"].get<float>();
        }
    }
}

void AmsBackendAfc::initialize_lanes(const std::vector<std::string>& lane_names) {
    int lane_count = static_cast<int>(lane_names.size());
    spdlog::info("[AMS AFC] Initializing {} lanes", lane_count);

    lane_names_ = lane_names;

    // Build lane name to index mapping
    lane_name_to_index_.clear();
    for (size_t i = 0; i < lane_names_.size(); ++i) {
        lane_name_to_index_[lane_names_[i]] = static_cast<int>(i);
    }

    // Create a single unit with all lanes (AFC units are typically treated as one logical unit)
    AmsUnit unit;
    unit.unit_index = 0;
    unit.name = "AFC Box Turtle";
    unit.gate_count = lane_count;
    unit.first_gate_global_index = 0;
    unit.connected = true;
    unit.has_encoder = false;        // AFC typically uses optical sensors, not encoders
    unit.has_toolhead_sensor = true; // Most AFC setups have toolhead sensor
    unit.has_gate_sensors = true;    // AFC has per-lane sensors

    // Initialize gates with defaults
    for (int i = 0; i < lane_count; ++i) {
        GateInfo gate;
        gate.gate_index = i;
        gate.global_index = i;
        gate.status = GateStatus::UNKNOWN;
        gate.mapped_tool = i; // Default 1:1 mapping
        gate.color_rgb = AMS_DEFAULT_GATE_COLOR;
        unit.gates.push_back(gate);
    }

    system_info_.units.clear();
    system_info_.units.push_back(unit);
    system_info_.total_gates = lane_count;

    // Initialize tool-to-gate mapping (1:1 default)
    system_info_.tool_to_gate_map.clear();
    system_info_.tool_to_gate_map.reserve(lane_count);
    for (int i = 0; i < lane_count; ++i) {
        system_info_.tool_to_gate_map.push_back(i);
    }

    lanes_initialized_ = true;
}

std::string AmsBackendAfc::get_lane_name(int gate_index) const {
    if (gate_index >= 0 && gate_index < static_cast<int>(lane_names_.size())) {
        return lane_names_[gate_index];
    }
    return "";
}

// ============================================================================
// Filament Operations
// ============================================================================

AmsError AmsBackendAfc::check_preconditions() const {
    if (!running_) {
        return AmsErrorHelper::not_connected("AFC backend not started");
    }

    if (system_info_.is_busy()) {
        return AmsErrorHelper::busy(ams_action_to_string(system_info_.action));
    }

    return AmsErrorHelper::success();
}

AmsError AmsBackendAfc::validate_gate_index(int gate_index) const {
    if (gate_index < 0 || gate_index >= system_info_.total_gates) {
        return AmsErrorHelper::invalid_gate(gate_index, system_info_.total_gates - 1);
    }
    return AmsErrorHelper::success();
}

AmsError AmsBackendAfc::execute_gcode(const std::string& gcode) {
    if (!api_) {
        return AmsErrorHelper::not_connected("MoonrakerAPI not available");
    }

    spdlog::info("[AMS AFC] Executing G-code: {}", gcode);

    // Execute G-code asynchronously via MoonrakerAPI
    api_->execute_gcode(
        gcode, []() { spdlog::debug("[AMS AFC] G-code executed successfully"); },
        [gcode](const MoonrakerError& err) {
            spdlog::error("[AMS AFC] G-code failed: {} - {}", gcode, err.message);
        });

    return AmsErrorHelper::success();
}

AmsError AmsBackendAfc::load_filament(int gate_index) {
    std::string lane_name;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        AmsError precondition = check_preconditions();
        if (!precondition) {
            return precondition;
        }

        AmsError gate_valid = validate_gate_index(gate_index);
        if (!gate_valid) {
            return gate_valid;
        }

        // Check if lane has filament available
        const auto* gate = system_info_.get_gate_global(gate_index);
        if (gate && gate->status == GateStatus::EMPTY) {
            return AmsErrorHelper::gate_not_available(gate_index);
        }

        lane_name = get_lane_name(gate_index);
        if (lane_name.empty()) {
            return AmsErrorHelper::invalid_gate(gate_index, system_info_.total_gates - 1);
        }
    }

    // Send AFC_LOAD LANE={name} command
    std::ostringstream cmd;
    cmd << "AFC_LOAD LANE=" << lane_name;

    spdlog::info("[AMS AFC] Loading from lane {} (gate {})", lane_name, gate_index);
    return execute_gcode(cmd.str());
}

AmsError AmsBackendAfc::unload_filament() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        AmsError precondition = check_preconditions();
        if (!precondition) {
            return precondition;
        }

        if (!system_info_.filament_loaded) {
            return AmsError(AmsResult::WRONG_STATE, "No filament loaded", "No filament to unload",
                            "Load filament first");
        }
    }

    spdlog::info("[AMS AFC] Unloading filament");
    return execute_gcode("AFC_UNLOAD");
}

AmsError AmsBackendAfc::select_gate(int gate_index) {
    std::string lane_name;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        AmsError precondition = check_preconditions();
        if (!precondition) {
            return precondition;
        }

        AmsError gate_valid = validate_gate_index(gate_index);
        if (!gate_valid) {
            return gate_valid;
        }

        lane_name = get_lane_name(gate_index);
        if (lane_name.empty()) {
            return AmsErrorHelper::invalid_gate(gate_index, system_info_.total_gates - 1);
        }
    }

    // AFC may not have a direct "select without load" command
    // Some AFC configurations use AFC_SELECT, others may require different approach
    std::ostringstream cmd;
    cmd << "AFC_SELECT LANE=" << lane_name;

    spdlog::info("[AMS AFC] Selecting lane {} (gate {})", lane_name, gate_index);
    return execute_gcode(cmd.str());
}

AmsError AmsBackendAfc::change_tool(int tool_number) {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        AmsError precondition = check_preconditions();
        if (!precondition) {
            return precondition;
        }

        if (tool_number < 0 ||
            tool_number >= static_cast<int>(system_info_.tool_to_gate_map.size())) {
            return AmsError(AmsResult::INVALID_TOOL,
                            "Tool " + std::to_string(tool_number) + " out of range",
                            "Invalid tool number", "Select a valid tool");
        }
    }

    // Send T{n} command for standard tool change
    std::ostringstream cmd;
    cmd << "T" << tool_number;

    spdlog::info("[AMS AFC] Tool change to T{}", tool_number);
    return execute_gcode(cmd.str());
}

// ============================================================================
// Recovery Operations
// ============================================================================

AmsError AmsBackendAfc::recover() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!running_) {
            return AmsErrorHelper::not_connected("AFC backend not started");
        }
    }

    // AFC may use AFC_RESET or AFC_RECOVER for error recovery
    spdlog::info("[AMS AFC] Initiating recovery");
    return execute_gcode("AFC_RESET");
}

AmsError AmsBackendAfc::home() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        AmsError precondition = check_preconditions();
        if (!precondition) {
            return precondition;
        }
    }

    spdlog::info("[AMS AFC] Homing AFC system");
    return execute_gcode("AFC_HOME");
}

AmsError AmsBackendAfc::cancel() {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!running_) {
            return AmsErrorHelper::not_connected("AFC backend not started");
        }

        if (system_info_.action == AmsAction::IDLE) {
            return AmsErrorHelper::success(); // Nothing to cancel
        }
    }

    // AFC may use AFC_ABORT or AFC_CANCEL to stop current operation
    spdlog::info("[AMS AFC] Cancelling current operation");
    return execute_gcode("AFC_ABORT");
}

// ============================================================================
// Configuration Operations
// ============================================================================

AmsError AmsBackendAfc::set_gate_info(int gate_index, const GateInfo& info) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (gate_index < 0 || gate_index >= system_info_.total_gates) {
        return AmsErrorHelper::invalid_gate(gate_index, system_info_.total_gates - 1);
    }

    auto* gate = system_info_.get_gate_global(gate_index);
    if (!gate) {
        return AmsErrorHelper::invalid_gate(gate_index, system_info_.total_gates - 1);
    }

    // Update local state
    gate->color_name = info.color_name;
    gate->color_rgb = info.color_rgb;
    gate->material = info.material;
    gate->brand = info.brand;
    gate->spoolman_id = info.spoolman_id;
    gate->spool_name = info.spool_name;
    gate->remaining_weight_g = info.remaining_weight_g;
    gate->total_weight_g = info.total_weight_g;
    gate->nozzle_temp_min = info.nozzle_temp_min;
    gate->nozzle_temp_max = info.nozzle_temp_max;
    gate->bed_temp = info.bed_temp;

    spdlog::info("[AMS AFC] Updated gate {} info: {} {}", gate_index, info.material,
                 info.color_name);

    emit_event(EVENT_GATE_CHANGED, std::to_string(gate_index));

    // AFC stores lane info in Moonraker database
    // This could be extended to persist changes via server.database.post_item
    // For now, we just update local state

    return AmsErrorHelper::success();
}

AmsError AmsBackendAfc::set_tool_mapping(int tool_number, int gate_index) {
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (tool_number < 0 ||
            tool_number >= static_cast<int>(system_info_.tool_to_gate_map.size())) {
            return AmsError(AmsResult::INVALID_TOOL,
                            "Tool " + std::to_string(tool_number) + " out of range",
                            "Invalid tool number", "");
        }

        if (gate_index < 0 || gate_index >= system_info_.total_gates) {
            return AmsErrorHelper::invalid_gate(gate_index, system_info_.total_gates - 1);
        }

        // Update local mapping
        system_info_.tool_to_gate_map[tool_number] = gate_index;

        // Update gate's mapped_tool reference
        for (auto& unit : system_info_.units) {
            for (auto& gate : unit.gates) {
                if (gate.mapped_tool == tool_number) {
                    gate.mapped_tool = -1; // Clear old mapping
                }
            }
        }
        auto* gate = system_info_.get_gate_global(gate_index);
        if (gate) {
            gate->mapped_tool = tool_number;
        }
    }

    // AFC may use a G-code command to set tool mapping
    // This varies by AFC version/configuration
    std::string lane_name = get_lane_name(gate_index);
    if (!lane_name.empty()) {
        std::ostringstream cmd;
        cmd << "AFC_MAP TOOL=" << tool_number << " LANE=" << lane_name;
        spdlog::info("[AMS AFC] Mapping T{} to lane {} (gate {})", tool_number, lane_name,
                     gate_index);
        return execute_gcode(cmd.str());
    }

    return AmsErrorHelper::success();
}
