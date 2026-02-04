// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyboard_shortcuts.h"

#include "../catch_amalgamated.hpp"

using namespace helix::input;

// ============================================================================
// KeyboardShortcuts unit tests
// ============================================================================

TEST_CASE("KeyboardShortcuts: basic key registration and firing", "[keyboard][input]") {
    KeyboardShortcuts shortcuts;
    int call_count = 0;

    shortcuts.register_key(10, [&call_count]() { call_count++; });

    SECTION("action fires when key pressed (edge trigger)") {
        // First process: key not pressed -> nothing happens
        shortcuts.process([](int) { return false; }, 0);
        REQUIRE(call_count == 0);

        // Key pressed -> action fires
        shortcuts.process([](int scancode) { return scancode == 10; }, 0);
        REQUIRE(call_count == 1);

        // Key still held -> no repeat
        shortcuts.process([](int scancode) { return scancode == 10; }, 0);
        REQUIRE(call_count == 1);

        // Key released
        shortcuts.process([](int) { return false; }, 0);
        REQUIRE(call_count == 1);

        // Key pressed again -> fires again
        shortcuts.process([](int scancode) { return scancode == 10; }, 0);
        REQUIRE(call_count == 2);
    }
}

TEST_CASE("KeyboardShortcuts: conditional key binding", "[keyboard][input]") {
    KeyboardShortcuts shortcuts;
    int call_count = 0;
    bool condition_enabled = false;

    shortcuts.register_key_if(
        20, [&call_count]() { call_count++; },
        [&condition_enabled]() { return condition_enabled; });

    SECTION("action does not fire when condition is false") {
        shortcuts.process([](int scancode) { return scancode == 20; }, 0);
        REQUIRE(call_count == 0);
    }

    SECTION("action fires when condition is true") {
        condition_enabled = true;
        shortcuts.process([](int scancode) { return scancode == 20; }, 0);
        REQUIRE(call_count == 1);
    }

    SECTION("condition checked on each press") {
        // Press with condition false
        shortcuts.process([](int scancode) { return scancode == 20; }, 0);
        REQUIRE(call_count == 0);

        // Release
        shortcuts.process([](int) { return false; }, 0);

        // Enable condition and press again
        condition_enabled = true;
        shortcuts.process([](int scancode) { return scancode == 20; }, 0);
        REQUIRE(call_count == 1);
    }
}

TEST_CASE("KeyboardShortcuts: modifier combo", "[keyboard][input]") {
    KeyboardShortcuts shortcuts;
    int call_count = 0;

    // KMOD_GUI is 0x0400 or 0x0800 (left/right)
    shortcuts.register_combo(0x0400, 30, [&call_count]() { call_count++; });

    SECTION("does not fire with just key") {
        shortcuts.process([](int scancode) { return scancode == 30; }, 0);
        REQUIRE(call_count == 0);
    }

    SECTION("does not fire with just modifier") {
        shortcuts.process([](int) { return false; }, 0x0400);
        REQUIRE(call_count == 0);
    }

    SECTION("fires with modifier + key") {
        shortcuts.process([](int scancode) { return scancode == 30; }, 0x0400);
        REQUIRE(call_count == 1);
    }

    SECTION("fires with superset of modifiers") {
        // Cmd+Shift+Q should still trigger Cmd+Q
        shortcuts.process([](int scancode) { return scancode == 30; }, 0x0400 | 0x0001);
        REQUIRE(call_count == 1);
    }
}

TEST_CASE("KeyboardShortcuts: multiple bindings", "[keyboard][input]") {
    KeyboardShortcuts shortcuts;
    int count_a = 0;
    int count_b = 0;

    shortcuts.register_key(40, [&count_a]() { count_a++; });
    shortcuts.register_key(50, [&count_b]() { count_b++; });

    SECTION("each binding independent") {
        shortcuts.process([](int scancode) { return scancode == 40; }, 0);
        REQUIRE(count_a == 1);
        REQUIRE(count_b == 0);

        shortcuts.process([](int scancode) { return scancode == 50; }, 0);
        REQUIRE(count_a == 1);
        REQUIRE(count_b == 1);
    }

    SECTION("both keys pressed simultaneously") {
        shortcuts.process([](int scancode) { return scancode == 40 || scancode == 50; }, 0);
        REQUIRE(count_a == 1);
        REQUIRE(count_b == 1);
    }
}

TEST_CASE("KeyboardShortcuts: clear removes all bindings", "[keyboard][input]") {
    KeyboardShortcuts shortcuts;
    int call_count = 0;

    shortcuts.register_key(60, [&call_count]() { call_count++; });

    // Verify it works
    shortcuts.process([](int scancode) { return scancode == 60; }, 0);
    REQUIRE(call_count == 1);

    // Clear and release key
    shortcuts.clear();
    shortcuts.process([](int) { return false; }, 0);

    // Press again - should not fire
    shortcuts.process([](int scancode) { return scancode == 60; }, 0);
    REQUIRE(call_count == 1); // still 1, not 2
}

TEST_CASE("KeyboardShortcuts: edge detection across clear", "[keyboard][input]") {
    KeyboardShortcuts shortcuts;
    int call_count = 0;

    shortcuts.register_key(70, [&call_count]() { call_count++; });

    // Press key
    shortcuts.process([](int scancode) { return scancode == 70; }, 0);
    REQUIRE(call_count == 1);

    // Clear and re-register while key still held
    shortcuts.clear();
    shortcuts.register_key(70, [&call_count]() { call_count++; });

    // Key still held - should not fire again (fresh registration = not pressed state)
    // Actually after clear, the new binding has no memory of previous state,
    // so it depends on implementation. Let's say fresh binding starts unpressed.
    shortcuts.process([](int scancode) { return scancode == 70; }, 0);
    // This is an edge case - new binding should see "key is pressed" on first check
    REQUIRE(call_count == 2); // fires because new binding sees edge
}
