// SPDX-License-Identifier: GPL-3.0-or-later

#include "led/led_controller.h"

#include "../catch_amalgamated.hpp"

TEST_CASE("LedController config: default values after init", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    REQUIRE(ctrl.last_color() == 0xFFFFFF);
    REQUIRE(ctrl.last_brightness() == 100);
    REQUIRE(ctrl.selected_strips().empty());
    // Default presets loaded during init->load_config
    REQUIRE(ctrl.color_presets().size() == 8);
    REQUIRE(ctrl.color_presets()[0] == 0xFFFFFF);
    REQUIRE(ctrl.color_presets()[1] == 0xFFD700);
    REQUIRE(ctrl.configured_macros().empty());

    ctrl.deinit();
}

TEST_CASE("LedController config: set and get last_color", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    ctrl.set_last_color(0xFF0000);
    REQUIRE(ctrl.last_color() == 0xFF0000);

    ctrl.set_last_color(0x00FF00);
    REQUIRE(ctrl.last_color() == 0x00FF00);

    ctrl.deinit();
}

TEST_CASE("LedController config: set and get last_brightness", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    ctrl.set_last_brightness(75);
    REQUIRE(ctrl.last_brightness() == 75);

    ctrl.set_last_brightness(0);
    REQUIRE(ctrl.last_brightness() == 0);

    ctrl.deinit();
}

TEST_CASE("LedController config: set and get selected_strips", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    std::vector<std::string> strips = {"neopixel chamber", "dotstar status"};
    ctrl.set_selected_strips(strips);
    REQUIRE(ctrl.selected_strips().size() == 2);
    REQUIRE(ctrl.selected_strips()[0] == "neopixel chamber");
    REQUIRE(ctrl.selected_strips()[1] == "dotstar status");

    ctrl.deinit();
}

TEST_CASE("LedController config: set and get color_presets", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    std::vector<uint32_t> presets = {0xFF0000, 0x00FF00, 0x0000FF};
    ctrl.set_color_presets(presets);
    REQUIRE(ctrl.color_presets().size() == 3);
    REQUIRE(ctrl.color_presets()[0] == 0xFF0000);
    REQUIRE(ctrl.color_presets()[2] == 0x0000FF);

    ctrl.deinit();
}

TEST_CASE("LedController config: configured macros round-trip", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    std::vector<helix::led::LedMacroInfo> macros;
    helix::led::LedMacroInfo m;
    m.display_name = "Cabinet Light";
    m.on_macro = "LIGHTS_ON";
    m.off_macro = "LIGHTS_OFF";
    m.toggle_macro = "";
    m.custom_actions = {{"Party", "LED_PARTY"}, {"Dim", "LED_DIM"}};
    macros.push_back(m);

    helix::led::LedMacroInfo m2;
    m2.display_name = "Status LED";
    m2.toggle_macro = "STATUS_TOGGLE";
    macros.push_back(m2);

    ctrl.set_configured_macros(macros);
    REQUIRE(ctrl.configured_macros().size() == 2);
    REQUIRE(ctrl.configured_macros()[0].display_name == "Cabinet Light");
    REQUIRE(ctrl.configured_macros()[0].on_macro == "LIGHTS_ON");
    REQUIRE(ctrl.configured_macros()[0].off_macro == "LIGHTS_OFF");
    REQUIRE(ctrl.configured_macros()[0].custom_actions.size() == 2);
    REQUIRE(ctrl.configured_macros()[0].custom_actions[0].first == "Party");
    REQUIRE(ctrl.configured_macros()[0].custom_actions[0].second == "LED_PARTY");
    REQUIRE(ctrl.configured_macros()[1].display_name == "Status LED");
    REQUIRE(ctrl.configured_macros()[1].toggle_macro == "STATUS_TOGGLE");

    ctrl.deinit();
}

TEST_CASE("LedController config: deinit resets config state to defaults", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    // Modify state
    ctrl.set_last_color(0xFF0000);
    ctrl.set_last_brightness(50);
    ctrl.set_selected_strips({"neopixel test"});
    ctrl.set_color_presets({0xABCDEF});

    helix::led::LedMacroInfo m;
    m.display_name = "Test";
    m.toggle_macro = "TEST_MACRO";
    ctrl.set_configured_macros({m});

    REQUIRE(ctrl.last_color() == 0xFF0000);
    REQUIRE(ctrl.last_brightness() == 50);
    REQUIRE(ctrl.selected_strips().size() == 1);
    REQUIRE(ctrl.color_presets().size() == 1);
    REQUIRE(ctrl.configured_macros().size() == 1);

    ctrl.deinit();

    // After deinit, re-init should restore defaults
    ctrl.init(nullptr, nullptr);
    REQUIRE(ctrl.last_color() == 0xFFFFFF);
    REQUIRE(ctrl.last_brightness() == 100);
    REQUIRE(ctrl.selected_strips().empty());
    REQUIRE(ctrl.color_presets().size() == 8); // Default presets restored
    REQUIRE(ctrl.configured_macros().empty());

    ctrl.deinit();
}

TEST_CASE("LedController config: default presets have correct values", "[led][config]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    auto& presets = ctrl.color_presets();
    REQUIRE(presets.size() == 8);
    REQUIRE(presets[0] == 0xFFFFFF); // White
    REQUIRE(presets[1] == 0xFFD700); // Gold
    REQUIRE(presets[2] == 0xFF6B35); // Orange
    REQUIRE(presets[3] == 0x4FC3F7); // Light Blue
    REQUIRE(presets[4] == 0xFF4444); // Red
    REQUIRE(presets[5] == 0x66BB6A); // Green
    REQUIRE(presets[6] == 0x9C27B0); // Purple
    REQUIRE(presets[7] == 0x00BCD4); // Cyan

    ctrl.deinit();
}
