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

#include "usb_backend.h"

#include "usb_backend_mock.h"

#ifdef __linux__
#include "usb_backend_linux.h"
#endif

#include <spdlog/spdlog.h>

std::unique_ptr<UsbBackend> UsbBackend::create(bool force_mock) {
    if (force_mock) {
        spdlog::info("[UsbBackend] Creating mock backend (force_mock=true)");
        return std::make_unique<UsbBackendMock>();
    }

#ifdef __linux__
    // Linux: Use inotify-based backend for real USB monitoring
    spdlog::info("[UsbBackend] Linux platform detected - using inotify backend");
    auto backend = std::make_unique<UsbBackendLinux>();
    UsbError result = backend->start();
    if (result.success()) {
        return backend;
    }

    // Fallback to mock if Linux backend fails
    spdlog::warn("[UsbBackend] Linux backend failed: {} - falling back to mock",
                 result.technical_msg);
    return std::make_unique<UsbBackendMock>();
#elif defined(__APPLE__)
    // macOS: Use mock backend for development
    // FSEvents-based backend can be added later for real monitoring
    spdlog::info("[UsbBackend] macOS platform detected - using mock backend");
    return std::make_unique<UsbBackendMock>();
#else
    // Unsupported platform: return mock
    spdlog::warn("[UsbBackend] Unknown platform - using mock backend");
    return std::make_unique<UsbBackendMock>();
#endif
}
