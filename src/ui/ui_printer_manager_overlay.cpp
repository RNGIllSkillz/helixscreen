// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui_printer_manager_overlay.h"

#include "ui_nav_manager.h"

#include "config.h"
#include "helix_version.h"
#include "printer_detector.h"
#include "printer_images.h"
#include "static_panel_registry.h"
#include "subject_debug_registry.h"
#include "wizard_config_paths.h"

#include <spdlog/spdlog.h>

#include <cstring>

// =============================================================================
// Global Instance
// =============================================================================

static std::unique_ptr<PrinterManagerOverlay> g_printer_manager_overlay;

PrinterManagerOverlay& get_printer_manager_overlay() {
    if (!g_printer_manager_overlay) {
        g_printer_manager_overlay = std::make_unique<PrinterManagerOverlay>();
        StaticPanelRegistry::instance().register_destroy(
            "PrinterManagerOverlay", []() { g_printer_manager_overlay.reset(); });
    }
    return *g_printer_manager_overlay;
}

void destroy_printer_manager_overlay() {
    g_printer_manager_overlay.reset();
}

// =============================================================================
// Constructor / Destructor
// =============================================================================

PrinterManagerOverlay::PrinterManagerOverlay() {
    std::memset(name_buf_, 0, sizeof(name_buf_));
    std::memset(model_buf_, 0, sizeof(model_buf_));
    std::memset(version_buf_, 0, sizeof(version_buf_));
}

PrinterManagerOverlay::~PrinterManagerOverlay() {
    if (lv_is_initialized()) {
        deinit_subjects_base(subjects_);
    }
}

// =============================================================================
// Subject Initialization
// =============================================================================

void PrinterManagerOverlay::init_subjects() {
    init_subjects_guarded([this]() {
        UI_MANAGED_SUBJECT_STRING(printer_manager_name_, name_buf_, "Unknown",
                                  "printer_manager_name", subjects_);
        UI_MANAGED_SUBJECT_STRING(printer_manager_model_, model_buf_, "", "printer_manager_model",
                                  subjects_);
        UI_MANAGED_SUBJECT_STRING(helix_version_, version_buf_, "0.0.0", "helix_version",
                                  subjects_);
    });
}

// =============================================================================
// Create
// =============================================================================

lv_obj_t* PrinterManagerOverlay::create(lv_obj_t* parent) {
    if (!create_overlay_from_xml(parent, "printer_manager_overlay")) {
        return nullptr;
    }

    // Find the printer image widget for programmatic image source setting
    printer_image_obj_ = lv_obj_find_by_name(overlay_root_, "pm_printer_image");

    return overlay_root_;
}

// =============================================================================
// Callbacks
// =============================================================================

void PrinterManagerOverlay::register_callbacks() {
    // Phase 1: No additional callbacks needed
    // Back button is handled by the overlay_panel component
}

void PrinterManagerOverlay::on_printer_manager_back_clicked(lv_event_t* e) {
    (void)e;
    if (g_printer_manager_overlay) {
        g_printer_manager_overlay->handle_back_clicked();
    }
}

void PrinterManagerOverlay::handle_back_clicked() {
    ui_nav_go_back();
}

// =============================================================================
// Lifecycle
// =============================================================================

void PrinterManagerOverlay::on_activate() {
    OverlayBase::on_activate();
    refresh_printer_info();
}

void PrinterManagerOverlay::on_deactivate() {
    OverlayBase::on_deactivate();
}

void PrinterManagerOverlay::cleanup() {
    OverlayBase::cleanup();
}

// =============================================================================
// Refresh Printer Info
// =============================================================================

void PrinterManagerOverlay::refresh_printer_info() {
    Config* config = Config::get_instance();
    if (!config) {
        spdlog::warn("[{}] Config not available", get_name());
        return;
    }

    // Printer name from config (user-given name, or fallback)
    std::string name = config->get<std::string>(helix::wizard::PRINTER_NAME, "");
    if (name.empty()) {
        name = "My Printer";
    }
    std::strncpy(name_buf_, name.c_str(), sizeof(name_buf_) - 1);
    name_buf_[sizeof(name_buf_) - 1] = '\0';
    lv_subject_copy_string(&printer_manager_name_, name_buf_);

    // Printer model/type from config
    std::string model = config->get<std::string>(helix::wizard::PRINTER_TYPE, "");
    std::strncpy(model_buf_, model.c_str(), sizeof(model_buf_) - 1);
    model_buf_[sizeof(model_buf_) - 1] = '\0';
    lv_subject_copy_string(&printer_manager_model_, model_buf_);

    // HelixScreen version
    const char* version = helix_version();
    std::strncpy(version_buf_, version, sizeof(version_buf_) - 1);
    version_buf_[sizeof(version_buf_) - 1] = '\0';
    lv_subject_copy_string(&helix_version_, version_buf_);

    spdlog::debug("[{}] Refreshed: name='{}', model='{}', version='{}'", get_name(), name_buf_,
                  model_buf_, version_buf_);

    // Update printer image programmatically (exception to declarative rule)
    if (printer_image_obj_ && !model.empty()) {
        std::string image_path = PrinterImages::get_image_path_for_name(model);
        lv_image_set_src(printer_image_obj_, image_path.c_str());
        spdlog::debug("[{}] Printer image: '{}' for '{}'", get_name(), image_path, model);
    }
}
