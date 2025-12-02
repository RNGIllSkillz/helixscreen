// SPDX-License-Identifier: GPL-3.0-or-later
//
// HelixScreen - Linux DRM/KMS Display Backend Implementation

#ifdef HELIX_DISPLAY_DRM

#include "display_backend_drm.h"
#include <spdlog/spdlog.h>

#include <lvgl.h>

// System includes for device access checks
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

DisplayBackendDRM::DisplayBackendDRM() = default;

DisplayBackendDRM::DisplayBackendDRM(const std::string& drm_device)
    : drm_device_(drm_device) {}

bool DisplayBackendDRM::is_available() const {
    struct stat st;

    // Check if DRM device exists
    if (stat(drm_device_.c_str(), &st) != 0) {
        spdlog::debug("DRM device {} not found", drm_device_);
        return false;
    }

    // Check if we can access it
    if (access(drm_device_.c_str(), R_OK | W_OK) != 0) {
        spdlog::debug("DRM device {} not accessible (need R/W permissions, check video group)",
                      drm_device_);
        return false;
    }

    return true;
}

lv_display_t* DisplayBackendDRM::create_display(int width, int height) {
    spdlog::info("Creating DRM display on {}", drm_device_);

    // LVGL's DRM driver
    display_ = lv_linux_drm_create();

    if (display_ == nullptr) {
        spdlog::error("Failed to create DRM display");
        return nullptr;
    }

    // Set the DRM device path
    lv_linux_drm_set_file(display_, drm_device_.c_str(), -1);

    spdlog::info("DRM display created: {}x{} on {}", width, height, drm_device_);
    return display_;
}

lv_indev_t* DisplayBackendDRM::create_input_pointer() {
    // Check environment variable for touch device override
    const char* env_device = std::getenv("HELIX_TOUCH_DEVICE");

    // Try libinput first (better for modern systems)
    // Then fall back to evdev

    // For DRM systems, we typically use libinput which handles device discovery
    spdlog::info("Creating libinput pointer device");

    pointer_ = lv_libinput_create(LV_INDEV_TYPE_POINTER, nullptr);

    if (pointer_ != nullptr) {
        spdlog::info("Libinput pointer device created");
        return pointer_;
    }

    spdlog::warn("Libinput failed, trying evdev fallback");

    // Fallback to evdev
    std::string touch_device = env_device ? env_device : "/dev/input/event0";
    pointer_ = lv_evdev_create(LV_INDEV_TYPE_POINTER, touch_device.c_str());

    if (pointer_ == nullptr) {
        spdlog::error("Failed to create input device");
        return nullptr;
    }

    spdlog::info("Evdev pointer device created on {}", touch_device);
    return pointer_;
}

#endif // HELIX_DISPLAY_DRM
