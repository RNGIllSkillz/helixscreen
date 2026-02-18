# Slicer-Preferred Progress Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Prefer slicer time-weighted progress (`display_status.progress` from M73) over file-position progress (`virtual_sdcard.progress`) for accurate print time estimation.

**Architecture:** Subscribe to Moonraker's `display_status` object. Track slicer progress alongside file progress. When slicer progress is actively updating (non-zero during a print), use it as the canonical progress source. Fall back to file progress transparently.

**Tech Stack:** C++ / Catch2 / nlohmann::json / LVGL subjects

**Design doc:** `docs/plans/2026-02-18-slicer-progress-design.md`

---

### Task 1: Write failing tests for slicer progress preference

**Files:**
- Create: `tests/unit/test_slicer_progress.cpp`

**Context:** Tests follow the same pattern as `tests/unit/test_printer_print_char.cpp`. Use `PrinterStateTestAccess::reset()` + `state.init_subjects(false)` for setup. Subject values are read via `lv_subject_get_int(state.get_print_progress_subject())`. JSON updates go through `state.update_from_status(json)`.

**Step 1: Write the test file**

```cpp
// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_slicer_progress.cpp
 * @brief Tests for slicer-preferred progress via display_status (Issue #122)
 *
 * When Klipper's display_status.progress is available and actively updating
 * (from M73 gcode commands), it should be preferred over virtual_sdcard.progress
 * for both displayed percentage and time estimation.
 */

#include "../test_helpers/printer_state_test_access.h"
#include "../ui_test_utils.h"
#include "app_globals.h"

#include "../catch_amalgamated.hpp"

using namespace helix;
using namespace helix::ui;
using json = nlohmann::json;

// ============================================================================
// Slicer Progress Preference Tests
// ============================================================================

TEST_CASE("Slicer progress: display_status.progress preferred when active",
          "[print][progress][slicer]") {
    lv_init_safe();
    PrinterState& state = get_printer_state();
    PrinterStateTestAccess::reset(state);
    state.init_subjects(false);

    SECTION("display_status.progress overrides virtual_sdcard when non-zero") {
        // Start a print
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        // File says 50%, slicer says 30% (slicer knows time != position)
        json status = {{"virtual_sdcard", {{"progress", 0.5}}},
                       {"display_status", {{"progress", 0.3}}}};
        state.update_from_status(status);

        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 30);
    }

    SECTION("display_status.progress activates on first non-zero value") {
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        // First update: slicer is 0 (M73 not yet encountered), use file progress
        json first = {{"virtual_sdcard", {{"progress", 0.05}}},
                      {"display_status", {{"progress", 0.0}}}};
        state.update_from_status(first);
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 5);

        // Second update: slicer now non-zero (M73 encountered), switch to slicer
        json second = {{"virtual_sdcard", {{"progress", 0.10}}},
                       {"display_status", {{"progress", 0.07}}}};
        state.update_from_status(second);
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 7);
    }

    SECTION("once slicer is active, stays active even if update lacks display_status") {
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        // Activate slicer progress
        json activate = {{"virtual_sdcard", {{"progress", 0.2}}},
                         {"display_status", {{"progress", 0.15}}}};
        state.update_from_status(activate);
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 15);

        // Next update only has virtual_sdcard (Moonraker batching)
        // Should NOT fall back to file progress — slicer is still active,
        // just keep the last slicer value (don't update progress from file)
        json file_only = {{"virtual_sdcard", {{"progress", 0.4}}}};
        state.update_from_status(file_only);
        // Progress should NOT jump to 40 — slicer is authoritative
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 15);
    }
}

// ============================================================================
// Fallback Tests — No Slicer Data
// ============================================================================

TEST_CASE("Slicer progress: falls back to virtual_sdcard when no slicer data",
          "[print][progress][slicer][fallback]") {
    lv_init_safe();
    PrinterState& state = get_printer_state();
    PrinterStateTestAccess::reset(state);
    state.init_subjects(false);

    SECTION("virtual_sdcard used when display_status never appears") {
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        json status = {{"virtual_sdcard", {{"progress", 0.5}}}};
        state.update_from_status(status);

        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 50);
    }

    SECTION("virtual_sdcard used when display_status.progress stays at 0") {
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        json status = {{"virtual_sdcard", {{"progress", 0.5}}},
                       {"display_status", {{"progress", 0.0}}}};
        state.update_from_status(status);

        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 50);
    }
}

// ============================================================================
// Reset Tests — New Print Clears Slicer State
// ============================================================================

TEST_CASE("Slicer progress: reset on new print", "[print][progress][slicer][reset]") {
    lv_init_safe();
    PrinterState& state = get_printer_state();
    PrinterStateTestAccess::reset(state);
    state.init_subjects(false);

    SECTION("slicer active flag resets when new print starts") {
        // First print: slicer was active
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        json slicer = {{"virtual_sdcard", {{"progress", 0.5}}},
                       {"display_status", {{"progress", 0.4}}}};
        state.update_from_status(slicer);
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 40);

        // Print completes
        json complete = {{"print_stats", {{"state", "complete"}}}};
        state.update_from_status(complete);

        // New print starts (standby -> printing transition triggers reset)
        json standby = {{"print_stats", {{"state", "standby"}}}};
        state.update_from_status(standby);
        json new_print = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(new_print);

        // This file has no M73 — should use virtual_sdcard
        json file_only = {{"virtual_sdcard", {{"progress", 0.1}}}};
        state.update_from_status(file_only);
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 10);
    }
}

// ============================================================================
// Terminal State Guard — Slicer Progress
// ============================================================================

TEST_CASE("Slicer progress: terminal state guard applies to slicer progress too",
          "[print][progress][slicer][guard]") {
    lv_init_safe();
    PrinterState& state = get_printer_state();
    PrinterStateTestAccess::reset(state);
    state.init_subjects(false);

    SECTION("slicer progress cannot go backward in COMPLETE state") {
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        json progress = {{"display_status", {{"progress", 1.0}}},
                         {"virtual_sdcard", {{"progress", 1.0}}}};
        state.update_from_status(progress);
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 100);

        json complete_state = {{"print_stats", {{"state", "complete"}}}};
        state.update_from_status(complete_state);

        // Moonraker may send display_status.progress=0 after complete
        json reset = {{"display_status", {{"progress", 0.0}}},
                      {"virtual_sdcard", {{"progress", 0.0}}}};
        state.update_from_status(reset);
        REQUIRE(lv_subject_get_int(state.get_print_progress_subject()) == 100);
    }
}

// ============================================================================
// Time Estimation — Slicer progress produces better estimates
// ============================================================================

TEST_CASE("Slicer progress: time estimation uses slicer progress",
          "[print][progress][slicer][time]") {
    lv_init_safe();
    PrinterState& state = get_printer_state();
    PrinterStateTestAccess::reset(state);
    state.init_subjects(false);

    SECTION("remaining time uses slicer progress percentage") {
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        // Slicer says 50%, file says 80% (dense gcode at end)
        // With 600s of print_duration at slicer-50%, remaining should be ~600s
        json status = {{"print_stats", {{"print_duration", 600.0}, {"total_duration", 650.0}}},
                       {"virtual_sdcard", {{"progress", 0.8}}},
                       {"display_status", {{"progress", 0.5}}}};
        state.update_from_status(status);

        int remaining = lv_subject_get_int(state.get_print_time_left_subject());
        // Formula: 600 * (100 - 50) / 50 = 600 seconds
        REQUIRE(remaining == 600);
    }

    SECTION("without slicer, file progress drives time estimation") {
        json printing = {{"print_stats", {{"state", "printing"}}}};
        state.update_from_status(printing);

        // Only file progress, no display_status
        // 600s at 80% → remaining = 600 * 20 / 80 = 150s
        json status = {{"print_stats", {{"print_duration", 600.0}, {"total_duration", 650.0}}},
                       {"virtual_sdcard", {{"progress", 0.8}}}};
        state.update_from_status(status);

        int remaining = lv_subject_get_int(state.get_print_time_left_subject());
        REQUIRE(remaining == 150);
    }
}
```

**Step 2: Run tests to verify they fail**

Run: `make test && ./build/bin/helix-tests "[slicer]" -v`
Expected: FAIL — display_status is not parsed yet, slicer progress fields don't exist

**Step 3: Commit failing tests**

```bash
git add tests/unit/test_slicer_progress.cpp
git commit -m "test: add failing tests for slicer-preferred progress (#122)"
```

---

### Task 2: Add member variables and subscription

**Files:**
- Modify: `include/printer_print_state.h` (add members near line 378)
- Modify: `src/api/moonraker_client.cpp` (add subscription near line 1446)

**Step 1: Add slicer progress tracking to PrinterPrintState**

In `include/printer_print_state.h`, after line 383 (`has_real_layer_data_`), add:

```cpp
    // Slicer progress from display_status (M73 gcode command)
    // When active, preferred over virtual_sdcard file-position progress
    double slicer_progress_ = 0.0;       // Raw 0.0-1.0 from display_status
    bool slicer_progress_active_ = false; // True once non-zero value seen during print
```

**Step 2: Add display_status to Moonraker subscription**

In `src/api/moonraker_client.cpp`, in `complete_discovery_subscription()`, after the core objects block (after `subscription_objects["system_stats"]` around line 1446), add:

```cpp
    subscription_objects["display_status"] = nullptr;
```

**Step 3: Build to verify compilation**

Run: `make -j`
Expected: PASS (compiles, no behavior change yet)

**Step 4: Commit**

```bash
git add include/printer_print_state.h src/api/moonraker_client.cpp
git commit -m "feat: add slicer progress tracking fields and display_status subscription (#122)"
```

---

### Task 3: Implement slicer progress parsing and preference logic

**Files:**
- Modify: `src/printer/printer_print_state.cpp` (lines 97-117 for reset, lines 298-338 for progress)

**Step 1: Reset slicer state in reset_for_new_print()**

In `reset_for_new_print()`, after `has_real_layer_data_ = false;` (line 105), add:

```cpp
    slicer_progress_ = 0.0;
    slicer_progress_active_ = false;
```

**Step 2: Parse display_status and implement preference logic**

In `update_from_status()`, BEFORE the existing `virtual_sdcard` block (before line 298), add display_status parsing:

```cpp
    // Parse slicer progress from display_status (M73 gcode command)
    if (status.contains("display_status")) {
        const auto& display = status["display_status"];
        if (display.contains("progress") && display["progress"].is_number()) {
            double raw = display["progress"].get<double>();
            slicer_progress_ = raw;
            if (raw > 0.0 && !slicer_progress_active_) {
                slicer_progress_active_ = true;
                spdlog::info("[PrinterPrintState] Slicer progress active (M73 detected)");
            }
        }
    }
```

Then modify the existing `virtual_sdcard` block (lines 298-338). Replace the progress_pct assignment and the subject set with slicer-aware logic:

```cpp
    // Update print progress (virtual_sdcard) - processed AFTER print_stats
    if (status.contains("virtual_sdcard")) {
        const auto& sdcard = status["virtual_sdcard"];

        if (sdcard.contains("progress") && sdcard["progress"].is_number()) {
            int file_progress_pct = helix::units::json_to_percent(sdcard, "progress");

            // Choose progress source: slicer (M73) when active, else file position
            int progress_pct = file_progress_pct;
            if (slicer_progress_active_) {
                progress_pct = static_cast<int>(slicer_progress_ * 100.0 + 0.5);
                if (progress_pct > 100) progress_pct = 100;
                if (progress_pct < 0) progress_pct = 0;
            }

            // Guard: Don't reset progress to 0 in terminal print states
            auto current_state = static_cast<PrintJobState>(lv_subject_get_int(&print_state_enum_));
            bool is_terminal_state = (current_state == PrintJobState::COMPLETE ||
                                      current_state == PrintJobState::CANCELLED ||
                                      current_state == PrintJobState::ERROR);

            int current_progress = lv_subject_get_int(&print_progress_);
            if (!is_terminal_state || progress_pct >= current_progress) {
                lv_subject_set_int(&print_progress_, progress_pct);
            }

            // Fallback: estimate current layer from progress
            if (!has_real_layer_data_ && !is_terminal_state && progress_pct > 0) {
                int total = lv_subject_get_int(&print_layer_total_);
                if (total > 0) {
                    int estimated = (progress_pct * total + 50) / 100;
                    if (estimated < 1) estimated = 1;
                    if (estimated > total) estimated = total;
                    int current = lv_subject_get_int(&print_layer_current_);
                    if (estimated != current) {
                        spdlog::debug("[LayerTracker] Estimated layer {}/{} from progress {}%",
                                      estimated, total, progress_pct);
                        lv_subject_set_int(&print_layer_current_, estimated);
                    }
                }
            }
        }
    }
```

Also handle the case where slicer is active but only `display_status` arrives without `virtual_sdcard` in the same message. Add AFTER the display_status block, BEFORE the virtual_sdcard block:

```cpp
    // When slicer progress is active and display_status updates without virtual_sdcard,
    // still update print_progress_ from the slicer value
    if (slicer_progress_active_ && status.contains("display_status") &&
        !status.contains("virtual_sdcard")) {
        int progress_pct = static_cast<int>(slicer_progress_ * 100.0 + 0.5);
        if (progress_pct > 100) progress_pct = 100;
        if (progress_pct < 0) progress_pct = 0;

        auto current_state = static_cast<PrintJobState>(lv_subject_get_int(&print_state_enum_));
        bool is_terminal_state = (current_state == PrintJobState::COMPLETE ||
                                  current_state == PrintJobState::CANCELLED ||
                                  current_state == PrintJobState::ERROR);
        int current_progress = lv_subject_get_int(&print_progress_);
        if (!is_terminal_state || progress_pct >= current_progress) {
            lv_subject_set_int(&print_progress_, progress_pct);
        }
    }
```

And when slicer is active but only `virtual_sdcard` arrives (no `display_status`), do NOT update `print_progress_` from file progress. Modify the virtual_sdcard block to skip the progress subject update when slicer is active and there's no display_status in this message:

The virtual_sdcard block should check: `if (slicer_progress_active_ && !status.contains("display_status"))` → skip the progress_pct subject set (but still do layer estimation using file_progress_pct).

**Step 3: Build and run tests**

Run: `make -j && make test && ./build/bin/helix-tests "[slicer]" -v`
Expected: All slicer tests PASS

**Step 4: Run full test suite**

Run: `make test-run`
Expected: All existing tests still PASS (no regressions — same behavior when display_status absent)

**Step 5: Commit**

```bash
git add src/printer/printer_print_state.cpp
git commit -m "feat: prefer slicer progress from display_status over file position (#122)"
```

---

### Task 4: Update mock client for testing

**Files:**
- Modify: `src/api/moonraker_client_mock_objects.cpp` (near line 349)

**Step 1: Ensure mock provides display_status updates during simulated prints**

The mock already generates `display_status` in initial status (line 349-351). Verify the mock's print simulation tick also sends `display_status.progress` updates. If it only sends `virtual_sdcard.progress` during tick, add `display_status.progress` alongside it so the `--test` mode exercises the slicer path.

Search for where mock sends progress updates during simulated printing and add `display_status` there too.

**Step 2: Build and run with mock**

Run: `make -j && ./build/bin/helix-screen --test -vv`
Expected: See `[PrinterPrintState] Slicer progress active (M73 detected)` in logs

**Step 3: Commit**

```bash
git add src/api/moonraker_client_mock_objects.cpp
git commit -m "feat: add display_status.progress to mock client print simulation (#122)"
```

---

### Task 5: Code review and final verification

**Step 1: Run full test suite**

Run: `make test-run`
Expected: ALL tests pass, including new `[slicer]` tests

**Step 2: Run with mock and verify log output**

Run: `./build/bin/helix-screen --test -vv 2>&1 | grep -i slicer`
Expected: Slicer progress active log message appears

**Step 3: Code review**

Use `superpowers:requesting-code-review` skill for comprehensive review.
