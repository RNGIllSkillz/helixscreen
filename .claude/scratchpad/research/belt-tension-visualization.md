# Belt Tension Visualization Research

**Date:** 2026-02-03
**Status:** Research complete, ready to design

## The Opportunity

Belt tension tuning is currently a manual, tedious process:
1. Pluck belt like a guitar string
2. Use smartphone app to measure frequency
3. Adjust tensioner
4. Repeat until frequency matches target

**HelixScreen could make this 10x better** by integrating directly with Klipper's accelerometer infrastructure.

---

## Key Discovery: Klipper Already Supports This!

### Built-in Belt Testing Commands

For CoreXY printers:
```gcode
TEST_RESONANCES AXIS=1,1 OUTPUT=raw_data   ; Test belt A
TEST_RESONANCES AXIS=1,-1 OUTPUT=raw_data  ; Test belt B
```

This generates CSV files with accelerometer data that can be processed into frequency graphs.

### Klippain Shake&Tune Approach

[Klippain Shake&Tune](https://github.com/Frix-x/klippain-shaketune) has a sophisticated `COMPARE_BELTS_RESPONSES` macro that:
- Uses diagonal movements to excite each belt individually
- Produces frequency profile graphs (Hz vs amplitude)
- Shows cross-belt comparison with similarity percentage
- Has "green zone" visualization for acceptable deviation

Parameters available:
| Parameter | Default | Function |
|-----------|---------|----------|
| FREQ_START | from config | Starting excitation frequency |
| FREQ_END | from config | Maximum excitation frequency |
| HZ_PER_SEC | 1 | Sweep rate |
| ACCEL_PER_HZ | from config | Acceleration magnitude per Hz |

---

## The Physics

Belt frequency relates to tension via:
```
f = 5.34 * sqrt(F) / L

where:
  f = frequency in Hz
  F = force in Newtons
  L = belt span length in meters
```

**Target frequencies:**
- Voron spec: ~110Hz for 150mm span
- Prusa CORE One: 96Hz upper / 92Hz lower
- A2 note on piano = 110Hz (easy reference!)

---

## Implementation Ideas for HelixScreen

### Option 1: One-Shot Belt Comparison (Lower effort)

1. Add "Belt Tension" item to calibration menu
2. Run `TEST_RESONANCES AXIS=1,1` and `AXIS=1,-1` via Moonraker
3. Parse CSV data, compute FFT
4. Display frequency profiles for both belts overlaid
5. Show similarity percentage and "good/needs adjustment" indicator
6. Display measured frequency vs target frequency

**Pros:** Uses existing infrastructure, reliable
**Cons:** Not interactive, requires full test cycle

### Option 2: Live Spectrum Analyzer (Higher effort, much cooler)

1. Stream accelerometer data in real-time via Moonraker
2. Show live frequency spectrum as user plucks belt
3. Peak detection shows current resonant frequency
4. Target frequency line for comparison
5. "Lock" feature to freeze reading when it stabilizes

**Pros:** Interactive, faster iteration, feels like magic
**Cons:** Requires streaming API, more complex

### Option 3: Automated Tension Wizard (Highest effort)

1. Guide user through step-by-step tensioning
2. Run motor excitation to vibrate belt
3. Measure response and compare to target
4. Tell user "tighten A belt" or "loosen B belt"
5. Repeat until matched

**Pros:** Most user-friendly
**Cons:** Complex, may not work for all printer configs

---

## Prior Art

### Klippain Shake&Tune
- Python-based, generates PNG graphs
- Runs as Klipper macros
- Excellent reference implementation
- [GitHub](https://github.com/Frix-x/klippain-shaketune)

### Smartphone Apps
- Spectroid (Android) - spectrum analyzer
- Gates Belt Tension app - manufacturer tool
- Guitar tuners (show frequency but not Hz directly)

### Feature Request in Klipper Community
- [Live frequency monitoring proposal](https://klipper.discourse.group/t/adxl-live-frequency-monitoring-for-belt-tuning/3240)
- People want real-time spectrum analyzer in the UI
- No one has implemented it yet in a touchscreen context

---

## Technical Questions to Answer

1. **Moonraker API:** Does Moonraker expose raw accelerometer data? Or just processed results?
2. **Streaming:** Can we get real-time accelerometer stream, or only batch results?
3. **CoreXY only?** Need to understand what makes sense for Cartesian, bedslinger, etc.
4. **Existing input shaper code:** We already have frequency response chart - can we reuse?

---

## Existing Infrastructure (can reuse!)

**We already have most of the pieces:**

1. **`ui_frequency_response_chart.cpp`** - Full frequency visualization widget
   - Multiple series support (perfect for A/B belt comparison)
   - Peak marking
   - Hardware adaptation for different platform tiers
   - Downsampling for embedded devices

2. **`input_shaper_calibrator.cpp`** - Calibration workflow orchestration
   - State machine pattern
   - Progress callbacks
   - Error handling

3. **`MoonrakerAPI::start_resonance_test()`** - Runs SHAPER_CALIBRATE
   - Would need variant for `TEST_RESONANCES AXIS=1,1` etc.

4. **`MoonrakerAPI::measure_axes_noise()`** - Accelerometer health check
   - Already exists, can reuse

---

## Implementation Plan

### Phase 1: One-Shot Belt Comparison (MVP)

**New API methods needed:**
```cpp
// In moonraker_api.h
void test_belt_resonance(const std::string& axis_spec,  // "1,1" or "1,-1"
                         BeltTestCallback on_complete,
                         ErrorCallback on_error);
```

**New files:**
- `src/calibration/belt_tension_calibrator.h/.cpp` - Orchestrator
- `src/ui/ui_panel_belt_tension.cpp` - UI panel
- `ui_xml/panel_belt_tension.xml` - Layout

**Workflow:**
1. User taps "Belt Tension" in calibration menu
2. System runs `TEST_RESONANCES AXIS=1,1 OUTPUT=raw_data`
3. Parse CSV, compute FFT
4. Repeat for `AXIS=1,-1`
5. Display both frequency profiles overlaid
6. Show peak frequencies and difference
7. Recommend "Tighten A" or "Loosen B" based on target

### Phase 2: Live Spectrum Analyzer (Stretch)

**Requires:**
- Bridge websocket connection to `adxl345/dump_adxl345`
- Real-time FFT in C++ (kissfft or similar)
- Streaming chart updates

**UX:**
- "Live Mode" toggle
- User plucks belt, sees frequency spike in real-time
- "Lock" button to freeze reading
- Target frequency indicator line

---

## Technical Questions Answered

1. **Moonraker API:** Yes! `adxl345/dump_adxl345` streams raw `[time, x, y, z]` data
2. **Streaming:** Available via bridge websocket (`ws://host/klippysocket`)
3. **CoreXY only?** Yes, `AXIS=1,1` and `1,-1` are CoreXY-specific. Cartesian would need different approach.
4. **Existing code:** `ui_frequency_response_chart` is directly reusable!

---

## Next Steps

1. [x] Research complete
2. [ ] Design UI mockup (what should the panel look like?)
3. [ ] Add `test_belt_resonance()` to MoonrakerAPI
4. [ ] Create `BeltTensionCalibrator` class
5. [ ] Build UI panel
6. [ ] Test on real CoreXY printer

---

## Sources

- [Klipper Measuring Resonances](https://www.klipper3d.org/Measuring_Resonances.html)
- [Klippain Shake&Tune Belt Comparison](https://github.com/Frix-x/klippain-shaketune/blob/main/docs/macros/compare_belts_responses.md)
- [Voron Belt Tensioning Docs](https://docs.vorondesign.com/tuning/secondary_printer_tuning.html)
- [Prusa Belt Frequency Guide](https://forum.prusa3d.com/forum/original-prusa-i3-mk3s-mk3-others-archive/use-free-span-frequency-hz-to-check-and-set-belt-tension/)
- [3D Distributed Belt Frequency Guide](https://3ddistributed.com/belt-frequency-and-tensioning/)
- [Klipper Live Monitoring Feature Request](https://klipper.discourse.group/t/adxl-live-frequency-monitoring-for-belt-tuning/3240)
