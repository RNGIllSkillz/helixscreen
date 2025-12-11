// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui_status_bar.h"

#include "ui_nav.h"
#include "ui_observer_guard.h"
#include "ui_panel_notification_history.h"
#include "ui_theme.h"

#include "app_globals.h"
#include "printer_state.h"

#include <spdlog/spdlog.h>

#include <cstring>

// Forward declaration for class-based API
NotificationHistoryPanel& get_global_notification_history_panel();

// ============================================================================
// Status Icon State Subjects (drive XML reactive bindings)
// ============================================================================
// These subjects expose computed state to XML. C++ computes WHAT state,
// XML bindings determine HOW to display it (colors, visibility, etc.)

// Printer icon state: 0=ready(green), 1=warning(orange), 2=error(red), 3=disconnected(gray)
static lv_subject_t printer_icon_state_subject;

// Network icon state: 0=connected(green), 1=connecting(orange), 2=disconnected(gray)
static lv_subject_t network_icon_state_subject;

// Notification badge: count (0 = hidden), text for display, severity for badge color
static lv_subject_t notification_count_subject;
static lv_subject_t notification_count_text_subject;
static lv_subject_t notification_severity_subject; // 0=info, 1=warning, 2=error

// Overlay backdrop visibility (for modal dimming)
static lv_subject_t overlay_backdrop_visible_subject;

// Track if subjects have been initialized
static bool subjects_initialized = false;

// Cached state for combined printer icon logic
static int32_t cached_connection_state = 0;
static int32_t cached_klippy_state = 0; // 0=READY, 1=STARTUP, 2=SHUTDOWN, 3=ERROR

// Notification count text buffer (for string subject)
static char notification_count_text_buf[8] = "0";

// RAII observer guards for automatic cleanup
static ObserverGuard s_network_observer;
static ObserverGuard s_connection_observer;
static ObserverGuard s_klippy_observer;

// Forward declaration
static void update_printer_icon_combined();

// Observer callback for network state changes
static void network_status_observer([[maybe_unused]] lv_observer_t* observer,
                                    lv_subject_t* subject) {
    int32_t network_state = lv_subject_get_int(subject);
    spdlog::debug("[StatusBar] Network observer fired! State: {}", network_state);

    // Map integer to NetworkStatus enum
    NetworkStatus status = static_cast<NetworkStatus>(network_state);
    ui_status_bar_update_network(status);
}

// Observer callback for printer connection state changes
static void printer_connection_observer([[maybe_unused]] lv_observer_t* observer,
                                        lv_subject_t* subject) {
    cached_connection_state = lv_subject_get_int(subject);
    spdlog::debug("[StatusBar] Connection state changed to: {}", cached_connection_state);
    update_printer_icon_combined();
}

// Observer callback for klippy state changes
static void klippy_state_observer([[maybe_unused]] lv_observer_t* observer, lv_subject_t* subject) {
    cached_klippy_state = lv_subject_get_int(subject);
    spdlog::debug("[StatusBar] Klippy state changed to: {}", cached_klippy_state);
    update_printer_icon_combined();
}

// Printer icon state constants (match XML visibility bindings)
enum PrinterIconState {
    PRINTER_STATE_READY = 0,       // Green - connected and klippy ready
    PRINTER_STATE_WARNING = 1,     // Orange - startup, reconnecting, was connected
    PRINTER_STATE_ERROR = 2,       // Red - klippy error/shutdown, connection failed
    PRINTER_STATE_DISCONNECTED = 3 // Gray - never connected
};

// Combined logic to update printer icon based on both connection and klippy state
// Now updates subject instead of directly styling the widget
static void update_printer_icon_combined() {
    // Klippy state takes precedence when connected
    // ConnectionState: 0=DISCONNECTED, 1=CONNECTING, 2=CONNECTED, 3=RECONNECTING, 4=FAILED
    // KlippyState: 0=READY, 1=STARTUP, 2=SHUTDOWN, 3=ERROR

    int32_t new_state;

    if (cached_connection_state == 2) { // CONNECTED to Moonraker
        // Check klippy state
        switch (cached_klippy_state) {
        case 1: // STARTUP (restarting)
            new_state = PRINTER_STATE_WARNING;
            spdlog::debug("[StatusBar] Klippy STARTUP -> printer state WARNING");
            break;
        case 2: // SHUTDOWN
        case 3: // ERROR
            new_state = PRINTER_STATE_ERROR;
            spdlog::debug("[StatusBar] Klippy SHUTDOWN/ERROR -> printer state ERROR");
            break;
        case 0: // READY
        default:
            new_state = PRINTER_STATE_READY;
            spdlog::debug("[StatusBar] Klippy READY -> printer state READY");
            break;
        }
    } else if (cached_connection_state == 4) { // FAILED
        new_state = PRINTER_STATE_ERROR;
        spdlog::debug("[StatusBar] Connection FAILED -> printer state ERROR");
    } else { // DISCONNECTED, CONNECTING, RECONNECTING
        if (get_printer_state().was_ever_connected()) {
            new_state = PRINTER_STATE_WARNING;
            spdlog::debug("[StatusBar] Disconnected (was connected) -> printer state WARNING");
        } else {
            new_state = PRINTER_STATE_DISCONNECTED;
            spdlog::debug("[StatusBar] Never connected -> printer state DISCONNECTED");
        }
    }

    // Update subject - XML bindings will handle the visual update
    if (subjects_initialized) {
        lv_subject_set_int(&printer_icon_state_subject, new_state);
    }
}

// Track notification panel to prevent multiple instances
static lv_obj_t* g_notification_panel_obj = nullptr;

// Event callback for notification history button
static void status_notification_history_clicked([[maybe_unused]] lv_event_t* e) {
    spdlog::info("[StatusBar] Notification history button CLICKED!");

    // Prevent multiple panel instances - if panel already exists and is visible, ignore click
    if (g_notification_panel_obj && lv_obj_is_valid(g_notification_panel_obj) &&
        !lv_obj_has_flag(g_notification_panel_obj, LV_OBJ_FLAG_HIDDEN)) {
        spdlog::debug("[StatusBar] Notification panel already visible, ignoring click");
        return;
    }

    lv_obj_t* parent = lv_screen_active();

    // Get panel instance and init subjects BEFORE creating XML
    // (subjects must be registered for XML bindings to work)
    auto& panel = get_global_notification_history_panel();
    if (!panel.are_subjects_initialized()) {
        panel.init_subjects();
    }

    // Clean up old panel if it exists but is hidden/invalid
    if (g_notification_panel_obj) {
        if (lv_obj_is_valid(g_notification_panel_obj)) {
            lv_obj_delete(g_notification_panel_obj);
        }
        g_notification_panel_obj = nullptr;
    }

    // Now create XML component - bindings can find the registered subjects
    lv_obj_t* panel_obj =
        static_cast<lv_obj_t*>(lv_xml_create(parent, "notification_history_panel", nullptr));
    if (!panel_obj) {
        spdlog::error("[StatusBar] Failed to create notification_history_panel from XML");
        return;
    }

    // Store reference for duplicate prevention
    g_notification_panel_obj = panel_obj;

    // Setup panel (wires buttons, refreshes list)
    panel.setup(panel_obj, parent);

    ui_nav_push_overlay(panel_obj);
}

void ui_status_bar_register_callbacks() {
    // Register notification history callback (must be called BEFORE app_layout XML is created)
    lv_xml_register_event_cb(NULL, "status_notification_history_clicked",
                             status_notification_history_clicked);
    spdlog::debug("[StatusBar] Event callbacks registered");
}

void ui_status_bar_init_subjects() {
    if (subjects_initialized) {
        spdlog::warn("[StatusBar] Subjects already initialized");
        return;
    }

    spdlog::debug("[StatusBar] Initializing status bar subjects...");

    // Initialize all subjects with default values
    // Printer starts disconnected (gray)
    lv_subject_init_int(&printer_icon_state_subject, PRINTER_STATE_DISCONNECTED);

    // Network starts disconnected (gray)
    lv_subject_init_int(&network_icon_state_subject, 2); // DISCONNECTED

    // Notification badge starts hidden (count = 0)
    lv_subject_init_int(&notification_count_subject, 0);
    lv_subject_init_pointer(&notification_count_text_subject, notification_count_text_buf);
    lv_subject_init_int(&notification_severity_subject, 0); // INFO

    // Overlay backdrop starts hidden
    lv_subject_init_int(&overlay_backdrop_visible_subject, 0);

    // Register subjects for XML binding
    lv_xml_register_subject(NULL, "printer_icon_state", &printer_icon_state_subject);
    lv_xml_register_subject(NULL, "network_icon_state", &network_icon_state_subject);
    lv_xml_register_subject(NULL, "notification_count", &notification_count_subject);
    lv_xml_register_subject(NULL, "notification_count_text", &notification_count_text_subject);
    lv_xml_register_subject(NULL, "notification_severity", &notification_severity_subject);
    lv_xml_register_subject(NULL, "overlay_backdrop_visible", &overlay_backdrop_visible_subject);

    subjects_initialized = true;
    spdlog::debug("[StatusBar] Subjects initialized and registered");
}

void ui_status_bar_init() {
    spdlog::debug("[StatusBar] ui_status_bar_init() called");

    // Ensure subjects are initialized (should be called before XML creation,
    // but this is a safety check)
    if (!subjects_initialized) {
        ui_status_bar_init_subjects();
    }

    // Observe network and printer states from PrinterState
    // These observers update our local subjects, which drive XML bindings
    PrinterState& printer_state = get_printer_state();

    // Network status observer (fires immediately with current value on registration)
    lv_subject_t* net_subject = printer_state.get_network_status_subject();
    spdlog::debug("[StatusBar] Registering observer on network_status_subject at {}",
                  (void*)net_subject);
    s_network_observer = ObserverGuard(net_subject, network_status_observer, nullptr);

    // Printer connection observer (fires immediately with current value on registration)
    lv_subject_t* conn_subject = printer_state.get_printer_connection_state_subject();
    spdlog::debug("[StatusBar] Registering observer on printer_connection_state_subject at {}",
                  (void*)conn_subject);
    s_connection_observer = ObserverGuard(conn_subject, printer_connection_observer, nullptr);

    // Klippy state observer (for RESTART/FIRMWARE_RESTART handling)
    lv_subject_t* klippy_subject = printer_state.get_klippy_state_subject();
    spdlog::debug("[StatusBar] Registering observer on klippy_state_subject at {}",
                  (void*)klippy_subject);
    s_klippy_observer = ObserverGuard(klippy_subject, klippy_state_observer, nullptr);

    // Note: Bell icon color is now set via XML (variant="secondary")
    // No widget lookup or styling needed here

    spdlog::debug("[StatusBar] Initialization complete");
}

// Network icon state constants (match XML visibility bindings)
enum NetworkIconState {
    NETWORK_STATE_CONNECTED = 0,   // Green
    NETWORK_STATE_CONNECTING = 1,  // Orange
    NETWORK_STATE_DISCONNECTED = 2 // Gray
};

void ui_status_bar_update_network(NetworkStatus status) {
    if (!subjects_initialized) {
        spdlog::warn("[StatusBar] Subjects not initialized, cannot update network icon");
        return;
    }

    // Map NetworkStatus enum to our icon state
    int32_t new_state;

    switch (status) {
    case NetworkStatus::CONNECTED:
        new_state = NETWORK_STATE_CONNECTED;
        spdlog::debug("[StatusBar] Network status CONNECTED -> state 0");
        break;
    case NetworkStatus::CONNECTING:
        new_state = NETWORK_STATE_CONNECTING;
        spdlog::debug("[StatusBar] Network status CONNECTING -> state 1");
        break;
    case NetworkStatus::DISCONNECTED:
    default:
        new_state = NETWORK_STATE_DISCONNECTED;
        spdlog::debug("[StatusBar] Network status DISCONNECTED -> state 2");
        break;
    }

    // Update subject - XML bindings will handle the visual update
    lv_subject_set_int(&network_icon_state_subject, new_state);
}

void ui_status_bar_update_printer(PrinterStatus status) {
    // Note: This function is now largely superseded by update_printer_icon_combined()
    // which uses the connection + klippy state observers for more accurate state.
    // Keeping this for API compatibility, but it just logs and delegates to the
    // combined logic which updates the printer_icon_state_subject.
    spdlog::debug("[StatusBar] ui_status_bar_update_printer() called with status={}",
                  static_cast<int>(status));

    // The combined observer-based logic in update_printer_icon_combined() handles
    // the actual subject update. This function is called less frequently now.
    // For now, just trigger a re-evaluation.
    update_printer_icon_combined();
}

// Notification severity constants (match XML visibility bindings for badge color)
enum NotificationSeverityState {
    NOTIFICATION_SEVERITY_INFO = 0,    // Blue badge
    NOTIFICATION_SEVERITY_WARNING = 1, // Orange badge
    NOTIFICATION_SEVERITY_ERROR = 2    // Red badge
};

void ui_status_bar_update_notification(NotificationStatus status) {
    if (!subjects_initialized) {
        spdlog::warn("[StatusBar] Subjects not initialized, cannot update notification");
        return;
    }

    // Map NotificationStatus enum to severity state for badge color
    int32_t severity;

    switch (status) {
    case NotificationStatus::ERROR:
        severity = NOTIFICATION_SEVERITY_ERROR;
        spdlog::debug("[StatusBar] Notification severity ERROR -> state 2");
        break;
    case NotificationStatus::WARNING:
        severity = NOTIFICATION_SEVERITY_WARNING;
        spdlog::debug("[StatusBar] Notification severity WARNING -> state 1");
        break;
    case NotificationStatus::INFO:
    case NotificationStatus::NONE:
    default:
        severity = NOTIFICATION_SEVERITY_INFO;
        spdlog::debug("[StatusBar] Notification severity INFO -> state 0");
        break;
    }

    // Update severity subject - XML bindings will update badge background color
    lv_subject_set_int(&notification_severity_subject, severity);
}

void ui_status_bar_update_notification_count(size_t count) {
    if (!subjects_initialized) {
        spdlog::trace("[StatusBar] Subjects not initialized, cannot update notification count");
        return;
    }

    // Update count subject (drives badge visibility: hidden when 0)
    lv_subject_set_int(&notification_count_subject, static_cast<int32_t>(count));

    // Update text subject for display
    snprintf(notification_count_text_buf, sizeof(notification_count_text_buf), "%zu", count);
    // Notify observers that the text changed (pointer didn't change, but content did)
    lv_subject_set_pointer(&notification_count_text_subject, notification_count_text_buf);

    spdlog::trace("[StatusBar] Notification count updated: {}", count);
}

void ui_status_bar_set_backdrop_visible(bool visible) {
    if (!subjects_initialized) {
        spdlog::warn("[StatusBar] Subjects not initialized, cannot set backdrop visibility");
        return;
    }

    lv_subject_set_int(&overlay_backdrop_visible_subject, visible ? 1 : 0);
    spdlog::debug("[StatusBar] Overlay backdrop visibility set to: {}", visible);
}
