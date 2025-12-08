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

#include "ams_backend.h"

#include "ams_backend_afc.h"
#include "ams_backend_happy_hare.h"
#include "ams_backend_mock.h"
#include "moonraker_api.h"
#include "runtime_config.h"

#include <spdlog/spdlog.h>

std::unique_ptr<AmsBackend> AmsBackend::create(AmsType detected_type) {
    // Check if mock mode is requested
    if (get_runtime_config().should_mock_ams()) {
        spdlog::info("AmsBackend: Creating mock backend (mock mode enabled)");
        return std::make_unique<AmsBackendMock>(4);
    }

    // Without API/client dependencies, we can only return mock backends
    switch (detected_type) {
    case AmsType::HAPPY_HARE:
        spdlog::warn("AmsBackend: Happy Hare detected but no API/client provided - using mock");
        return std::make_unique<AmsBackendMock>(4);

    case AmsType::AFC:
        spdlog::warn("AmsBackend: AFC detected but no API/client provided - using mock");
        return std::make_unique<AmsBackendMock>(4);

    case AmsType::NONE:
    default:
        spdlog::debug("AmsBackend: No AMS detected");
        return nullptr;
    }
}

std::unique_ptr<AmsBackend> AmsBackend::create(AmsType detected_type, MoonrakerAPI* api,
                                               MoonrakerClient* client) {
    // Check if mock mode is requested
    if (get_runtime_config().should_mock_ams()) {
        spdlog::info("AmsBackend: Creating mock backend (mock mode enabled)");
        return std::make_unique<AmsBackendMock>(4);
    }

    switch (detected_type) {
    case AmsType::HAPPY_HARE:
        if (!api || !client) {
            spdlog::error("AmsBackend: Happy Hare requires MoonrakerAPI and MoonrakerClient");
            return nullptr;
        }
        spdlog::info("AmsBackend: Creating Happy Hare backend");
        return std::make_unique<AmsBackendHappyHare>(api, client);

    case AmsType::AFC:
        if (!api || !client) {
            spdlog::error("AmsBackend: AFC requires MoonrakerAPI and MoonrakerClient");
            return nullptr;
        }
        spdlog::info("AmsBackend: Creating AFC backend");
        return std::make_unique<AmsBackendAfc>(api, client);

    case AmsType::NONE:
    default:
        spdlog::debug("AmsBackend: No AMS detected");
        return nullptr;
    }
}
