# Slicer-Preferred Progress via `display_status` (Issue #122)

## Problem

HelixScreen shows inaccurate print time remaining because it uses `virtual_sdcard.progress` (file-position-based) for time extrapolation. File position doesn't correlate linearly with time — a 3-hour print can show as 8 hours.

## Solution

Subscribe to Klipper's `display_status.progress`, which reflects M73 commands embedded by slicers. M73 progress is time-weighted and much more accurate. When available, prefer it over `virtual_sdcard.progress` for both displayed percentage and time estimation.

## Design

### Data Flow

```
Moonraker WebSocket
  ├── display_status.progress  (0.0-1.0, slicer time-weighted via M73)
  └── virtual_sdcard.progress  (0.0-1.0, file-position based)
        ↓
PrinterPrintState::update_from_status()
  → if slicer progress active: use display_status.progress
  → else: fall back to virtual_sdcard.progress
        ↓
print_progress_ subject (int 0-100, unchanged interface)
        ↓
Existing time estimation formula (unchanged)
  remaining = print_time * (100 - progress) / progress
```

### Detection Logic

- Track `slicer_progress_` (double, raw 0.0-1.0)
- Track `slicer_progress_active_` (bool) — true when `display_status.progress` changes to non-zero during active print
- Reset both on print start
- Once active, stays active for duration of print (M73 embedded throughout gcode)

### Files Changed

1. `moonraker_client.cpp` — Add `display_status` to subscription list
2. `moonraker_client_mock_objects.cpp` — Activate existing mock data
3. `printer_print_state.h` — Add `slicer_progress_` and `slicer_progress_active_` members
4. `printer_print_state.cpp` — Parse `display_status.progress`, prefer over `virtual_sdcard.progress`
5. Tests — Verify fallback, slicer preference, reset on print start

### What Stays the Same

- `print_progress_` subject type/range (int 0-100) — all UI bindings untouched
- Time estimation formula
- Slicer blending at low progress (<5%)
- All XML
- Print status panel observers

### UX Decisions

- No user-facing setting — auto-prefer slicer progress when available
- Slicer progress used for both displayed % and time estimation (consistency)
- Silent fallback to current method when display_status unavailable or stuck at 0

### Related

- Issue #124: Display `display_status.message` on print status screen (separate feature, shares subscription)
