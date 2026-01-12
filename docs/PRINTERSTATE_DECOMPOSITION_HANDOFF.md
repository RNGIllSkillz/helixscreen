# PrinterState God Class Decomposition - Handoff Document

**Created**: 2026-01-11
**Last Updated**: 2026-01-12
**Status**: IN PROGRESS - 4 domains extracted, Print State characterization tests DONE

## Quick Resume

```bash
# 1. Go to the worktree
cd /Users/pbrown/Code/Printing/helixscreen-printer-state-decomp

# 2. Verify worktree is up to date
git fetch origin && git status

# 3. Build
make -j && make test-build

# 4. Run characterization tests (should all pass - 117 tests, 904 assertions)
./build/bin/helix-tests "[characterization]"

# 5. Continue with next step: Extract PrinterPrintState class
```

---

## Project Overview

### Goal
Decompose the 2808-line `PrinterState` god class (86 subjects across 11+ domains) into focused, testable domain state classes.

### Source Reference
- **REFACTOR_PLAN.md**: `docs/REFACTOR_PLAN.md` section 1.1 "PrinterState God Class Decomposition"
- **Branch**: `feature/printer-state-decomposition`
- **Worktree**: `/Users/pbrown/Code/Printing/helixscreen-printer-state-decomp`

---

## Progress Summary

### Completed Extractions

| Domain | Class | Subjects | Tests | Assertions | Commit |
|--------|-------|----------|-------|------------|--------|
| Temperature | `PrinterTemperatureState` | 4 | 26 | 145 | 36dec0bb |
| Motion | `PrinterMotionState` | 8 | 21 | 165 | dfa92d60 |
| LED | `PrinterLedState` | 6 | 18 | 146 | ee5ac704 |
| Fan | `PrinterFanState` | 2 static + dynamic | 26 | 118 | ee5ac704 |
| Print (tests only) | — | 17 | 26 | 330 | Pending |
| **Total** | | **37+** | **117** | **904** | |

### Next Steps

| Step | Description | Status |
|------|-------------|--------|
| 1 | Write Print State characterization tests | ✅ DONE |
| 2 | Extract PrinterPrintState class | ⏳ NEXT |
| 3 | Continue with remaining domains... | Pending |

---

## Extracted Components

### 1. PrinterTemperatureState (4 subjects)
**File**: `include/printer_temperature_state.h`, `src/printer/printer_temperature_state.cpp`

| Subject | Type | Storage |
|---------|------|---------|
| `extruder_temp_` | int | Centidegrees (205.3°C → 2053) |
| `extruder_target_` | int | Centidegrees |
| `bed_temp_` | int | Centidegrees |
| `bed_target_` | int | Centidegrees |

### 2. PrinterMotionState (8 subjects)
**File**: `include/printer_motion_state.h`, `src/printer/printer_motion_state.cpp`

| Subject | Type | Storage |
|---------|------|---------|
| `position_x_`, `position_y_`, `position_z_` | int | mm (truncated from float) |
| `homed_axes_` | string | "xyz", "xy", "" etc |
| `speed_factor_`, `flow_factor_` | int | Percentage (100 = 100%) |
| `gcode_z_offset_` | int | Microns (mm × 1000) |
| `pending_z_offset_delta_` | int | Microns (accumulated) |

**Additional methods**: `add_pending_z_offset_delta()`, `get_pending_z_offset_delta()`, `has_pending_z_offset_adjustment()`, `clear_pending_z_offset_delta()`

### 3. PrinterLedState (6 subjects)
**File**: `include/printer_led_state.h`, `src/printer/printer_led_state.cpp`

| Subject | Type | Storage |
|---------|------|---------|
| `led_state_` | int | 0=off, 1=on |
| `led_r_`, `led_g_`, `led_b_`, `led_w_` | int | 0-255 (from 0.0-1.0 JSON) |
| `led_brightness_` | int | 0-100 (max channel × 100 / 255) |

**Additional methods**: `set_tracked_led()`, `get_tracked_led()`, `has_tracked_led()`

### 4. PrinterFanState (2 static + dynamic)
**File**: `include/printer_fan_state.h`, `src/printer/printer_fan_state.cpp`

| Subject | Type | Storage |
|---------|------|---------|
| `fan_speed_` | int | 0-100% (main part-cooling fan) |
| `fans_version_` | int | Increments on fan list changes |
| `fan_speed_subjects_[name]` | map<string, unique_ptr<lv_subject_t>> | Per-fan speeds |

**Also includes**: `FanType` enum, `FanInfo` struct, `init_fans()`, `update_fan_speed()`, `classify_fan_type()`, `is_fan_controllable()`

---

## Remaining Domains

### PRINT STATE DOMAIN (17 subjects) - **NEXT**

The largest and most complex domain. Includes:

| Subject | Type | Purpose |
|---------|------|---------|
| `print_progress_` | int | 0-100% |
| `print_filename_` | string | Current file |
| `print_state_` | string | "standby", "printing", etc |
| `print_state_enum_` | int | PrintJobState enum |
| `print_active_` | int | 0/1 boolean |
| `print_outcome_` | int | PrintOutcome enum |
| `print_show_progress_` | int | Visibility flag |
| `print_thumbnail_path_` | string | Thumbnail file path |
| Layer tracking | int × 2 | current_layer_, total_layers_ |
| Time tracking | int × 3 | print_duration_, time_remaining_, eta_ |
| Print start phases | int × 2 | print_start_phase_, preheat_complete_ |
| Workflow flags | int × 2 | print_can_pause_, print_can_cancel_ |

### Other Remaining Domains

- **Capabilities** (12 subjects): `printer_has_qgl_`, `printer_has_z_tilt_`, etc.
- **Network/Connection** (6 subjects): `printer_connection_state_`, `klippy_state_`, etc.
- **Hardware Validation** (11 subjects): `hardware_has_issues_`, etc.
- **Calibration/Config** (8 subjects)
- **Plugin Status** (3 subjects)
- **Composite Visibility** (5 subjects)
- **Firmware Retraction** (4 subjects)
- **Manual Probe** (2 subjects)
- **Excluded Objects** (2 subjects)
- **Versions** (2 subjects)
- **Kinematics** (1 subject)
- **Motors** (1 subject)

---

## Extraction Pattern

All extractions follow this pattern:

### 1. Write Characterization Tests First
```cpp
TEST_CASE("Domain characterization: ...", "[characterization][domain]") {
    lv_init_safe();
    PrinterState& state = get_printer_state();
    state.reset_for_testing();
    state.init_subjects(false);
    // Test current behavior
}
```

### 2. Create Domain State Class
```cpp
// include/printer_domain_state.h
namespace helix {
class PrinterDomainState {
public:
    void init_subjects(bool register_xml = true);
    void deinit_subjects();
    void update_from_status(const nlohmann::json& status);
    void reset_for_testing();

    lv_subject_t* get_subject_name() { return &subject_name_; }

private:
    SubjectManager subjects_;
    bool subjects_initialized_ = false;
    lv_subject_t subject_name_{};
};
}
```

### 3. Update PrinterState to Delegate
```cpp
// In PrinterState
lv_subject_t* get_subject_name() {
    return domain_state_.get_subject_name();
}
private:
    helix::PrinterDomainState domain_state_;
```

### 4. Verify All Tests Pass
- Characterization tests
- Existing printer tests
- Full test suite

### 5. Review and Commit

---

## Key Files

### Worktree
```
/Users/pbrown/Code/Printing/helixscreen-printer-state-decomp/
```

### Extracted Components
```
include/printer_temperature_state.h
include/printer_motion_state.h
include/printer_led_state.h
include/printer_fan_state.h
src/printer/printer_temperature_state.cpp
src/printer/printer_motion_state.cpp
src/printer/printer_led_state.cpp
src/printer/printer_fan_state.cpp
```

### Test Files
```
tests/unit/test_printer_temperature_char.cpp  # 26 tests
tests/unit/test_printer_motion_char.cpp       # 21 tests
tests/unit/test_printer_led_char.cpp          # 18 tests
tests/unit/test_printer_fan_char.cpp          # 26 tests
tests/unit/test_printer_print_char.cpp        # 26 tests
```

---

## Commands Reference

```bash
# Build
cd /Users/pbrown/Code/Printing/helixscreen-printer-state-decomp
make -j

# Run all characterization tests
./build/bin/helix-tests "[characterization]"

# Run specific domain tests
./build/bin/helix-tests "[temperature]"
./build/bin/helix-tests "[motion]"
./build/bin/helix-tests "[led]"
./build/bin/helix-tests "[fan]"
./build/bin/helix-tests "[print]"

# Run full printer state tests
./build/bin/helix-tests "[printer]" "~[slow]"

# Full test suite
make test-run

# Git status
git status --short
git log --oneline -5
```

---

## Resumption Checklist

When resuming this work:

- [ ] Read this document
- [ ] `cd /Users/pbrown/Code/Printing/helixscreen-printer-state-decomp`
- [ ] `git fetch origin && git status`
- [ ] `make -j` (verify builds)
- [ ] `./build/bin/helix-tests "[characterization]"` (91 tests, 574 assertions)
- [ ] Continue with Print State characterization tests
- [ ] Follow test-first: write characterization tests BEFORE extraction
- [ ] Use `/delegate` for implementation work
- [ ] Use `/review` before committing

---

## Session History

### 2026-01-11 Session 1
1. Created worktree `feature/printer-state-decomposition`
2. Analyzed PrinterState: 86 subjects across 11+ domains
3. Created temperature characterization tests (26 tests)
4. **PAUSED** - Next: Extract PrinterTemperatureState class

### 2026-01-11/12 Session 2
1. Extracted PrinterTemperatureState (4 subjects)
2. Wrote motion characterization tests (21 tests)
3. Extracted PrinterMotionState (8 subjects)
4. Wrote LED characterization tests (18 tests)
5. Extracted PrinterLedState (6 subjects)
6. Wrote Fan characterization tests (26 tests)
7. Extracted PrinterFanState (2 static + dynamic per-fan subjects)
8. All 91 characterization tests passing (574 assertions)
9. **PAUSED** - Next: Print State domain (17 subjects)

### 2026-01-12 Session 3
1. Wrote Print State characterization tests (26 tests, 330 assertions)
   - Core state: print_state_, print_state_enum_, print_active_, print_outcome_
   - File info: print_filename_, print_display_filename_, print_thumbnail_path_
   - Progress: print_progress_, print_show_progress_
   - Layers: print_layer_current_, print_layer_total_
   - Time: print_duration_, print_time_left_
   - Start phases: print_start_phase_, print_start_message_, print_start_progress_
   - Workflow: print_in_progress_, can_start_new_print()
2. Critical behaviors tested: terminal state persistence, progress guards, derived subjects
3. All 117 characterization tests passing (904 assertions)
4. **PAUSED** - Next: Extract PrinterPrintState class

---

## Commits on Branch

```
ee5ac704 refactor(printer): extract PrinterLedState and PrinterFanState
dfa92d60 refactor(printer): extract PrinterMotionState from PrinterState
36dec0bb refactor(printer): extract PrinterTemperatureState from PrinterState
cf74706d test(char): add temperature domain characterization tests
```

---

HANDOFF: PrinterState God Class Decomposition - 4 domains extracted, Print State characterization tests DONE
