// SPDX-License-Identifier: GPL-3.0-or-later

#include "led/led_controller.h"
#include "printer_discovery.h"

#include "../catch_amalgamated.hpp"

TEST_CASE("LedController singleton access", "[led]") {
    auto& ctrl = helix::led::LedController::instance();
    auto& ctrl2 = helix::led::LedController::instance();
    REQUIRE(&ctrl == &ctrl2);
}

TEST_CASE("LedController init and deinit", "[led]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit(); // Clean state

    REQUIRE(!ctrl.is_initialized());
    ctrl.init(nullptr, nullptr); // null api/client for testing
    REQUIRE(ctrl.is_initialized());
    ctrl.deinit();
    REQUIRE(!ctrl.is_initialized());
}

TEST_CASE("LedController has_any_backend empty", "[led]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    REQUIRE(!ctrl.has_any_backend());
    REQUIRE(ctrl.available_backends().empty());

    ctrl.deinit();
}

TEST_CASE("LedController discover_from_hardware populates native backend", "[led]") {
    // Use PrinterDiscovery to populate
    helix::PrinterDiscovery discovery;
    nlohmann::json objects = nlohmann::json::array(
        {"neopixel chamber_light", "dotstar status_led", "led case_light", "extruder"});
    discovery.parse_objects(objects);

    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);
    ctrl.discover_from_hardware(discovery);

    REQUIRE(ctrl.has_any_backend());
    REQUIRE(ctrl.native().is_available());
    REQUIRE(ctrl.native().strips().size() == 3);

    // Check strip details
    auto& strips = ctrl.native().strips();
    REQUIRE(strips[0].id == "neopixel chamber_light");
    REQUIRE(strips[0].name == "Chamber Light");
    REQUIRE(strips[0].supports_color == true);
    REQUIRE(strips[0].supports_white == true);

    REQUIRE(strips[1].id == "dotstar status_led");
    REQUIRE(strips[1].name == "Status Led");
    REQUIRE(strips[1].supports_white == true);

    REQUIRE(strips[2].id == "led case_light");
    REQUIRE(strips[2].name == "Case Light");
    REQUIRE(strips[2].supports_white == false);

    // Other backends should be empty
    REQUIRE(!ctrl.effects().is_available());
    REQUIRE(!ctrl.wled().is_available());
    REQUIRE(!ctrl.macro().is_available());

    auto backends = ctrl.available_backends();
    REQUIRE(backends.size() == 1);
    REQUIRE(backends[0] == helix::led::LedBackendType::NATIVE);

    ctrl.deinit();
}

TEST_CASE("LedBackendType enum values", "[led]") {
    REQUIRE(static_cast<int>(helix::led::LedBackendType::NATIVE) == 0);
    REQUIRE(static_cast<int>(helix::led::LedBackendType::LED_EFFECT) == 1);
    REQUIRE(static_cast<int>(helix::led::LedBackendType::WLED) == 2);
    REQUIRE(static_cast<int>(helix::led::LedBackendType::MACRO) == 3);
}

TEST_CASE("LedStripInfo struct", "[led]") {
    helix::led::LedStripInfo info;
    info.name = "Chamber Light";
    info.id = "neopixel chamber_light";
    info.backend = helix::led::LedBackendType::NATIVE;
    info.supports_color = true;
    info.supports_white = true;

    REQUIRE(info.name == "Chamber Light");
    REQUIRE(info.id == "neopixel chamber_light");
    REQUIRE(info.backend == helix::led::LedBackendType::NATIVE);
    REQUIRE(info.supports_color);
    REQUIRE(info.supports_white);
}

TEST_CASE("LedEffectBackend icon hint mapping", "[led]") {
    using helix::led::LedEffectBackend;

    REQUIRE(LedEffectBackend::icon_hint_for_effect("breathing") == "air");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("pulse_slow") == "air");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("fire_effect") == "local_fire_department");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("flame") == "local_fire_department");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("rainbow_chase") == "palette");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("comet_tail") == "fast_forward");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("chase_effect") == "fast_forward");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("static_white") == "lightbulb");
    REQUIRE(LedEffectBackend::icon_hint_for_effect("my_custom_effect") == "auto_awesome");
}

TEST_CASE("LedEffectBackend display name conversion", "[led]") {
    using helix::led::LedEffectBackend;

    REQUIRE(LedEffectBackend::display_name_for_effect("led_effect breathing") == "Breathing");
    REQUIRE(LedEffectBackend::display_name_for_effect("led_effect fire_effect") == "Fire Effect");
    REQUIRE(LedEffectBackend::display_name_for_effect("rainbow_chase") == "Rainbow Chase");
    REQUIRE(LedEffectBackend::display_name_for_effect("") == "");
}

TEST_CASE("NativeBackend strip management", "[led]") {
    helix::led::NativeBackend backend;

    REQUIRE(!backend.is_available());
    REQUIRE(backend.strips().empty());

    helix::led::LedStripInfo strip;
    strip.name = "Test Strip";
    strip.id = "neopixel test";
    strip.backend = helix::led::LedBackendType::NATIVE;
    strip.supports_color = true;
    strip.supports_white = false;

    backend.add_strip(strip);
    REQUIRE(backend.is_available());
    REQUIRE(backend.strips().size() == 1);

    backend.clear();
    REQUIRE(!backend.is_available());
}

TEST_CASE("MacroBackend macro management", "[led]") {
    helix::led::MacroBackend backend;

    REQUIRE(!backend.is_available());

    helix::led::LedMacroInfo macro;
    macro.display_name = "Cabinet Light";
    macro.on_macro = "LIGHTS_ON";
    macro.off_macro = "LIGHTS_OFF";
    macro.custom_actions = {{"Party Mode", "LED_PARTY"}};

    backend.add_macro(macro);
    REQUIRE(backend.is_available());
    REQUIRE(backend.macros().size() == 1);
    REQUIRE(backend.macros()[0].display_name == "Cabinet Light");
    REQUIRE(backend.macros()[0].custom_actions.size() == 1);

    backend.clear();
    REQUIRE(!backend.is_available());
}

TEST_CASE("LedController deinit clears all backends", "[led]") {
    auto& ctrl = helix::led::LedController::instance();
    ctrl.deinit();
    ctrl.init(nullptr, nullptr);

    // Add some data
    helix::led::LedStripInfo strip;
    strip.name = "Test";
    strip.id = "neopixel test";
    strip.backend = helix::led::LedBackendType::NATIVE;
    strip.supports_color = true;
    strip.supports_white = false;
    ctrl.native().add_strip(strip);

    helix::led::LedEffectInfo effect;
    effect.name = "led_effect test";
    effect.display_name = "Test";
    effect.icon_hint = "auto_awesome";
    ctrl.effects().add_effect(effect);

    REQUIRE(ctrl.has_any_backend());

    ctrl.deinit();

    REQUIRE(!ctrl.has_any_backend());
    REQUIRE(ctrl.native().strips().empty());
    REQUIRE(ctrl.effects().effects().empty());
}

TEST_CASE("LedController: selected_strips can hold WLED strip IDs", "[led][controller]") {
    auto& controller = helix::led::LedController::instance();
    controller.deinit();

    // Set selected strips to a WLED-style ID
    controller.set_selected_strips({"wled_printer_led"});
    REQUIRE(controller.selected_strips().size() == 1);
    REQUIRE(controller.selected_strips()[0] == "wled_printer_led");

    // Can switch back to native
    controller.set_selected_strips({"neopixel chamber_light"});
    REQUIRE(controller.selected_strips()[0] == "neopixel chamber_light");
}
