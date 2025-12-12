// Copyright 2025 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui_panel_calibration_pid.h"

#include "ui_event_safety.h"
#include "ui_nav.h"
#include "ui_theme.h"

#include "moonraker_client.h"

#include <spdlog/spdlog.h>

#include <cstdio>
#include <memory>

// ============================================================================
// SUBJECT INITIALIZATION
// ============================================================================

void PIDCalibrationPanel::init_subjects() {
    if (subjects_initialized_) {
        spdlog::warn("[PIDCal] Subjects already initialized");
        return;
    }

    spdlog::debug("[PIDCal] Initializing subjects");

    // Initialize state subject (0=IDLE, 1=CALIBRATING, 2=SAVING, 3=COMPLETE, 4=ERROR)
    lv_subject_init_int(&pid_cal_state_, static_cast<int>(State::IDLE));
    lv_xml_register_subject(nullptr, "pid_cal_state", &pid_cal_state_);

    subjects_initialized_ = true;
    spdlog::debug("[PIDCal] Subjects initialized");
}

// ============================================================================
// CALLBACK REGISTRATION
// ============================================================================

void PIDCalibrationPanel::register_callbacks() {
    spdlog::debug("[PIDCal] Registering event callbacks");

    // Register all button callbacks (wired via XML event_cb elements)
    lv_xml_register_event_cb(nullptr, "on_pid_heater_extruder_clicked", on_heater_extruder_clicked);
    lv_xml_register_event_cb(nullptr, "on_pid_heater_bed_clicked", on_heater_bed_clicked);
    lv_xml_register_event_cb(nullptr, "on_pid_temp_up", on_temp_up);
    lv_xml_register_event_cb(nullptr, "on_pid_temp_down", on_temp_down);
    lv_xml_register_event_cb(nullptr, "on_pid_start_clicked", on_start_clicked);
    lv_xml_register_event_cb(nullptr, "on_pid_abort_clicked", on_abort_clicked);
    lv_xml_register_event_cb(nullptr, "on_pid_done_clicked", on_done_clicked);
    lv_xml_register_event_cb(nullptr, "on_pid_retry_clicked", on_retry_clicked);

    spdlog::debug("[PIDCal] Event callbacks registered");
}

// ============================================================================
// SETUP
// ============================================================================

void PIDCalibrationPanel::setup(lv_obj_t* panel, lv_obj_t* parent_screen, MoonrakerClient* client) {
    panel_ = panel;
    parent_screen_ = parent_screen;
    client_ = client;

    if (!panel_) {
        spdlog::error("[PIDCal] NULL panel");
        return;
    }

    // Find widgets in idle state (for dynamic updates)
    btn_heater_extruder_ = lv_obj_find_by_name(panel_, "btn_heater_extruder");
    btn_heater_bed_ = lv_obj_find_by_name(panel_, "btn_heater_bed");
    temp_display_ = lv_obj_find_by_name(panel_, "temp_display");
    temp_hint_ = lv_obj_find_by_name(panel_, "temp_hint");

    // Find widgets in calibrating state
    calibrating_heater_ = lv_obj_find_by_name(panel_, "calibrating_heater");
    current_temp_display_ = lv_obj_find_by_name(panel_, "current_temp_display");

    // Find widgets in complete state
    pid_kp_ = lv_obj_find_by_name(panel_, "pid_kp");
    pid_ki_ = lv_obj_find_by_name(panel_, "pid_ki");
    pid_kd_ = lv_obj_find_by_name(panel_, "pid_kd");

    // Find error message label
    error_message_ = lv_obj_find_by_name(panel_, "error_message");

    // NOTE: Event handlers are wired via XML <event_cb> elements
    // and registered globally in register_callbacks()

    // Set initial state (subject binding controls visibility)
    set_state(State::IDLE);
    update_heater_selection();
    update_temp_display();
    update_temp_hint();

    spdlog::info("[PIDCal] Setup complete");
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

void PIDCalibrationPanel::set_state(State new_state) {
    spdlog::debug("[PIDCal] State change: {} -> {}", static_cast<int>(state_),
                  static_cast<int>(new_state));
    state_ = new_state;

    // Update subject - XML bind_flag_if_not_eq bindings control view visibility
    lv_subject_set_int(&pid_cal_state_, static_cast<int>(new_state));
}

// ============================================================================
// UI UPDATES
// ============================================================================

void PIDCalibrationPanel::update_heater_selection() {
    if (!btn_heater_extruder_ || !btn_heater_bed_)
        return;

    // Use background color to indicate selection
    lv_color_t selected_color = ui_theme_get_color("primary_color");
    lv_color_t neutral_color = ui_theme_get_color("theme_grey");

    if (selected_heater_ == Heater::EXTRUDER) {
        lv_obj_set_style_bg_color(btn_heater_extruder_, selected_color, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn_heater_bed_, neutral_color, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(btn_heater_extruder_, neutral_color, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn_heater_bed_, selected_color, LV_PART_MAIN);
    }
}

void PIDCalibrationPanel::update_temp_display() {
    if (!temp_display_)
        return;

    char buf[16];
    snprintf(buf, sizeof(buf), "%d°C", target_temp_);
    lv_label_set_text(temp_display_, buf);
}

void PIDCalibrationPanel::update_temp_hint() {
    if (!temp_hint_)
        return;

    if (selected_heater_ == Heater::EXTRUDER) {
        lv_label_set_text(temp_hint_, "Recommended: 200°C for extruder");
    } else {
        lv_label_set_text(temp_hint_, "Recommended: 60°C for heated bed");
    }
}

void PIDCalibrationPanel::update_temperature(float current, float target) {
    if (!current_temp_display_)
        return;

    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f°C / %.0f°C", current, target);
    lv_label_set_text(current_temp_display_, buf);
}

// ============================================================================
// GCODE COMMANDS
// ============================================================================

void PIDCalibrationPanel::send_pid_calibrate() {
    if (!client_) {
        spdlog::error("[PIDCal] No Moonraker client");
        on_calibration_result(false, 0, 0, 0, "No printer connection");
        return;
    }

    const char* heater_name = (selected_heater_ == Heater::EXTRUDER) ? "extruder" : "heater_bed";

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "PID_CALIBRATE HEATER=%s TARGET=%d", heater_name, target_temp_);

    spdlog::info("[PIDCal] Sending: {}", cmd);
    int result = client_->gcode_script(cmd);
    if (result <= 0) {
        spdlog::error("[PIDCal] Failed to send PID_CALIBRATE");
        on_calibration_result(false, 0, 0, 0, "Failed to start calibration");
    }

    // Update calibrating state label
    if (calibrating_heater_) {
        const char* label = (selected_heater_ == Heater::EXTRUDER) ? "Extruder PID Tuning"
                                                                   : "Heated Bed PID Tuning";
        lv_label_set_text(calibrating_heater_, label);
    }

    // For demo purposes, simulate completion after a delay
    // In real implementation, this would be triggered by Moonraker events
    lv_timer_t* timer = lv_timer_create(
        [](lv_timer_t* t) {
            auto* self = static_cast<PIDCalibrationPanel*>(lv_timer_get_user_data(t));
            if (self && self->get_state() == State::CALIBRATING) {
                // Simulate successful calibration with typical values
                self->on_calibration_result(true, 22.865f, 1.292f, 101.178f);
            }
            lv_timer_delete(t);
        },
        5000, this); // 5 second delay to simulate calibration
    lv_timer_set_repeat_count(timer, 1);
}

void PIDCalibrationPanel::send_save_config() {
    if (!client_)
        return;

    spdlog::info("[PIDCal] Sending SAVE_CONFIG");
    int result = client_->gcode_script("SAVE_CONFIG");
    if (result <= 0) {
        spdlog::error("[PIDCal] Failed to send SAVE_CONFIG");
        on_calibration_result(false, 0, 0, 0, "Failed to save configuration");
        return;
    }

    // Simulate save completing
    lv_timer_t* timer = lv_timer_create(
        [](lv_timer_t* t) {
            auto* self = static_cast<PIDCalibrationPanel*>(lv_timer_get_user_data(t));
            if (self && self->get_state() == State::SAVING) {
                self->set_state(State::COMPLETE);
            }
            lv_timer_delete(t);
        },
        2000, this);
    lv_timer_set_repeat_count(timer, 1);
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void PIDCalibrationPanel::handle_heater_extruder_clicked() {
    if (state_ != State::IDLE)
        return;

    spdlog::debug("[PIDCal] Extruder selected");
    selected_heater_ = Heater::EXTRUDER;
    target_temp_ = EXTRUDER_DEFAULT_TEMP;
    update_heater_selection();
    update_temp_display();
    update_temp_hint();
}

void PIDCalibrationPanel::handle_heater_bed_clicked() {
    if (state_ != State::IDLE)
        return;

    spdlog::debug("[PIDCal] Heated bed selected");
    selected_heater_ = Heater::BED;
    target_temp_ = BED_DEFAULT_TEMP;
    update_heater_selection();
    update_temp_display();
    update_temp_hint();
}

void PIDCalibrationPanel::handle_temp_up() {
    if (state_ != State::IDLE)
        return;

    int max_temp = (selected_heater_ == Heater::EXTRUDER) ? EXTRUDER_MAX_TEMP : BED_MAX_TEMP;

    if (target_temp_ < max_temp) {
        target_temp_ += 5;
        update_temp_display();
    }
}

void PIDCalibrationPanel::handle_temp_down() {
    if (state_ != State::IDLE)
        return;

    int min_temp = (selected_heater_ == Heater::EXTRUDER) ? EXTRUDER_MIN_TEMP : BED_MIN_TEMP;

    if (target_temp_ > min_temp) {
        target_temp_ -= 5;
        update_temp_display();
    }
}

void PIDCalibrationPanel::handle_start_clicked() {
    spdlog::debug("[PIDCal] Start clicked");
    set_state(State::CALIBRATING);
    send_pid_calibrate();
}

void PIDCalibrationPanel::handle_abort_clicked() {
    spdlog::debug("[PIDCal] Abort clicked");
    // Send TURN_OFF_HEATERS to abort
    if (client_) {
        client_->gcode_script("TURN_OFF_HEATERS");
    }
    set_state(State::IDLE);
}

void PIDCalibrationPanel::handle_done_clicked() {
    spdlog::debug("[PIDCal] Done clicked");
    set_state(State::IDLE);
    ui_nav_go_back();
}

void PIDCalibrationPanel::handle_retry_clicked() {
    spdlog::debug("[PIDCal] Retry clicked");
    set_state(State::IDLE);
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

void PIDCalibrationPanel::on_calibration_result(bool success, float kp, float ki, float kd,
                                                const std::string& error_message) {
    if (success) {
        // Store results
        result_kp_ = kp;
        result_ki_ = ki;
        result_kd_ = kd;

        // Update display
        if (pid_kp_) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.3f", kp);
            lv_label_set_text(pid_kp_, buf);
        }
        if (pid_ki_) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.3f", ki);
            lv_label_set_text(pid_ki_, buf);
        }
        if (pid_kd_) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.3f", kd);
            lv_label_set_text(pid_kd_, buf);
        }

        // Save config (will transition to COMPLETE when done)
        set_state(State::SAVING);
        send_save_config();
    } else {
        if (error_message_) {
            lv_label_set_text(error_message_, error_message.c_str());
        }
        set_state(State::ERROR);
    }
}

// ============================================================================
// STATIC TRAMPOLINES (XML event_cb callbacks)
// ============================================================================

void PIDCalibrationPanel::on_heater_extruder_clicked(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_heater_extruder_clicked");
    get_global_pid_cal_panel().handle_heater_extruder_clicked();
    LVGL_SAFE_EVENT_CB_END();
}

void PIDCalibrationPanel::on_heater_bed_clicked(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_heater_bed_clicked");
    get_global_pid_cal_panel().handle_heater_bed_clicked();
    LVGL_SAFE_EVENT_CB_END();
}

void PIDCalibrationPanel::on_temp_up(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_temp_up");
    get_global_pid_cal_panel().handle_temp_up();
    LVGL_SAFE_EVENT_CB_END();
}

void PIDCalibrationPanel::on_temp_down(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_temp_down");
    get_global_pid_cal_panel().handle_temp_down();
    LVGL_SAFE_EVENT_CB_END();
}

void PIDCalibrationPanel::on_start_clicked(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_start_clicked");
    get_global_pid_cal_panel().handle_start_clicked();
    LVGL_SAFE_EVENT_CB_END();
}

void PIDCalibrationPanel::on_abort_clicked(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_abort_clicked");
    get_global_pid_cal_panel().handle_abort_clicked();
    LVGL_SAFE_EVENT_CB_END();
}

void PIDCalibrationPanel::on_done_clicked(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_done_clicked");
    get_global_pid_cal_panel().handle_done_clicked();
    LVGL_SAFE_EVENT_CB_END();
}

void PIDCalibrationPanel::on_retry_clicked(lv_event_t* e) {
    LV_UNUSED(e);
    LVGL_SAFE_EVENT_CB_BEGIN("[PIDCal] on_retry_clicked");
    get_global_pid_cal_panel().handle_retry_clicked();
    LVGL_SAFE_EVENT_CB_END();
}

// ============================================================================
// GLOBAL INSTANCE
// ============================================================================

static std::unique_ptr<PIDCalibrationPanel> g_pid_cal_panel;

PIDCalibrationPanel& get_global_pid_cal_panel() {
    if (!g_pid_cal_panel) {
        g_pid_cal_panel = std::make_unique<PIDCalibrationPanel>();
    }
    return *g_pid_cal_panel;
}

// ============================================================================
// INITIALIZATION (must be called before XML creation)
// ============================================================================

void ui_panel_calibration_pid_register_callbacks() {
    // Register event callbacks for XML event_cb elements
    PIDCalibrationPanel::register_callbacks();

    // Initialize subjects BEFORE XML creation (bindings resolve at parse time)
    get_global_pid_cal_panel().init_subjects();

    spdlog::debug("[PIDCal] Registered callbacks and initialized subjects");
}
