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

#include <string>
#include <functional>
#include <vector>
#include <memory>

/**
 * @brief WiFi network information
 */
struct WiFiNetwork {
    std::string ssid;           ///< Network name (SSID)
    int signal_strength;        ///< Signal strength (0-100 percentage)
    bool is_secured;            ///< True if network requires password
    std::string security_type;  ///< Security type ("WPA2", "WPA3", "WEP", "Open")

    WiFiNetwork() : signal_strength(0), is_secured(false) {}

    WiFiNetwork(const std::string& ssid_, int strength, bool secured, const std::string& security = "")
        : ssid(ssid_), signal_strength(strength), is_secured(secured), security_type(security) {}
};

/**
 * @brief Abstract WiFi backend interface
 *
 * Provides a clean, platform-agnostic API for WiFi operations.
 * Concrete implementations handle platform-specific details:
 * - WifiBackendWpaSupplicant: Linux wpa_supplicant integration
 * - WifiBackendMock: Simulator mode with fake data
 *
 * Design principles:
 * - Hide all backend-specific formats/commands from WiFiManager
 * - Provide async operations with event-based completion
 * - Thread-safe operations where needed
 * - Clean error handling with meaningful messages
 */
class WifiBackend {
public:
    virtual ~WifiBackend() = default;

    /**
     * @brief Connection status information
     */
    struct ConnectionStatus {
        bool connected;           ///< True if connected to a network
        std::string ssid;         ///< Connected network name
        std::string bssid;        ///< Access point MAC address
        std::string ip_address;   ///< Current IP address
        int signal_strength;      ///< Signal strength (0-100%)
    };

    // ========================================================================
    // Lifecycle Management
    // ========================================================================

    /**
     * @brief Initialize and start the WiFi backend
     *
     * Establishes connection to underlying WiFi system (wpa_supplicant, mock, etc.)
     * and starts any background processing threads.
     *
     * @return true if initialization succeeded
     */
    virtual bool start() = 0;

    /**
     * @brief Stop the WiFi backend
     *
     * Cleanly shuts down background threads and connections.
     */
    virtual void stop() = 0;

    /**
     * @brief Check if backend is currently running/initialized
     *
     * @return true if backend is active and ready for operations
     */
    virtual bool is_running() const = 0;

    // ========================================================================
    // Event System
    // ========================================================================

    /**
     * @brief Register callback for WiFi events
     *
     * Events are delivered asynchronously and may arrive from background threads.
     * Ensure thread safety in callback implementations.
     *
     * Standard event types:
     * - "SCAN_COMPLETE" - Network scan finished
     * - "CONNECTED" - Successfully connected to network
     * - "DISCONNECTED" - Disconnected from network
     * - "AUTH_FAILED" - Authentication failed (wrong password, etc.)
     *
     * @param name Event type identifier
     * @param callback Handler function
     */
    virtual void register_event_callback(const std::string& name,
                                        std::function<void(const std::string&)> callback) = 0;

    // ========================================================================
    // Network Scanning
    // ========================================================================

    /**
     * @brief Trigger network scan (async)
     *
     * Initiates scan for available WiFi networks. Results delivered via
     * "SCAN_COMPLETE" event. Use get_scan_results() to retrieve networks.
     *
     * @return true if scan initiated successfully
     */
    virtual bool trigger_scan() = 0;

    /**
     * @brief Get scan results
     *
     * Returns networks discovered by the most recent scan.
     * Call after receiving "SCAN_COMPLETE" event for up-to-date results.
     *
     * @return Vector of WiFiNetwork structs (sorted by signal strength, descending)
     */
    virtual std::vector<WiFiNetwork> get_scan_results() = 0;

    // ========================================================================
    // Connection Management
    // ========================================================================

    /**
     * @brief Connect to network (async)
     *
     * Initiates connection to specified network. Results delivered via
     * "CONNECTED" event (success) or "AUTH_FAILED"/"DISCONNECTED" (failure).
     *
     * @param ssid Network name
     * @param password Password (empty string for open networks)
     * @return true if connection attempt initiated successfully
     */
    virtual bool connect_network(const std::string& ssid, const std::string& password) = 0;

    /**
     * @brief Disconnect from current network
     *
     * @return true if disconnect succeeded
     */
    virtual bool disconnect_network() = 0;

    // ========================================================================
    // Status Queries
    // ========================================================================

    /**
     * @brief Get current connection status
     *
     * @return ConnectionStatus struct with current state
     */
    virtual ConnectionStatus get_status() = 0;

    // ========================================================================
    // Factory Methods
    // ========================================================================

    /**
     * @brief Create appropriate backend for current platform
     *
     * - Linux: WifiBackendWpaSupplicant (real wpa_supplicant integration)
     * - macOS: WifiBackendMock (simulator with fake data)
     *
     * @return Unique pointer to backend instance
     */
    static std::unique_ptr<WifiBackend> create();
};