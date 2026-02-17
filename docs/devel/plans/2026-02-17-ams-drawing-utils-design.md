# AMS Drawing Utils — Shared Drawing Code Refactor

**Date:** 2026-02-17
**Status:** Design approved, pending implementation

## Problem

AMS drawing/styling code is duplicated across 6 files. The mini AMS widget
(home panel), overview cards (AMS panel), detail slot widget, spool canvas,
and main AMS panel all reimplement the same patterns: color manipulation,
pulse animations, error badges, bar creation, status-line styling, logo
fallback, and container boilerplate. Bug fixes and visual changes must be
applied in multiple places, and the mini widget's visuals have drifted from
the overview cards.

## Goals

1. **DRY** — Extract shared drawing code into `ams_drawing_utils.h/.cpp`
2. **Visual alignment** — Mini widget adopts overview card style (loaded =
   wider border, severity-aware error coloring)
3. **Keep layout ownership per-widget** — Each widget still owns its own
   layout logic (stacked rows, cards, grids). Only the per-slot rendering
   and common utilities are shared.

## New File: `src/ui/ams_drawing_utils.h` / `.cpp`

All functions in `namespace ams_draw`.

### A. Color Utilities

```cpp
lv_color_t lighten_color(lv_color_t c, uint8_t amount);
lv_color_t darken_color(lv_color_t c, uint8_t amount);
lv_color_t blend_color(lv_color_t c1, lv_color_t c2, float factor);
```

**Replaces:** `ui_ams_mini_status.cpp:61`, `ui_spool_canvas.cpp:53-70`,
`ui_ams_slot.cpp:162`, inline lightening in `ui_panel_ams_overview.cpp:595`.

### B. Severity Color Mapping

```cpp
lv_color_t severity_color(SlotError::Severity severity);
// ERROR → "danger", WARNING → "warning", INFO → "text_muted"
```

**Replaces:** 5+ if/else chains in `ui_ams_slot.cpp:508-516`,
`ui_panel_ams_overview.cpp:457-461, 500-504, 622-625`,
`ui_ams_mini_status.cpp:167-176` (after visual alignment).

### C. Worst Unit Severity

```cpp
SlotError::Severity worst_unit_severity(const AmsUnit& unit);
```

**Replaces:** `ui_panel_ams_overview.cpp:159-167`. Already well-contained
but belongs in shared utils for reuse.

### D. Pulse Animation

```cpp
// Constants
constexpr int32_t PULSE_SCALE_MIN = 180;
constexpr int32_t PULSE_SCALE_MAX = 256;
constexpr int32_t PULSE_SAT_MIN = 80;
constexpr int32_t PULSE_SAT_MAX = 255;
constexpr uint32_t PULSE_DURATION_MS = 800;

void start_pulse(lv_obj_t* dot, lv_color_t base_color);
void stop_pulse(lv_obj_t* dot);
```

**Replaces:** `ui_panel_ams_overview.cpp:72-135` (badge_scale_anim_cb,
badge_color_anim_cb, start_badge_pulse, stop_badge_pulse) and
`ui_ams_slot.cpp:415-494` (error_dot_scale_anim_cb, error_dot_color_anim_cb,
start_error_dot_pulse, stop_error_dot_pulse). These are structurally
identical — same constants, same animation pattern. Minor differences in
shadow intensity are unified to the ams_slot values.

### E. Error Badge

```cpp
lv_obj_t* create_error_badge(lv_obj_t* parent, int32_t size);
void update_error_badge(lv_obj_t* badge, bool has_error,
                        SlotError::Severity severity, bool animate);
```

**Replaces:** Badge creation in `ui_panel_ams_overview.cpp:441-469`, update
logic at `:498-514`, and `apply_slot_error` in `ui_ams_slot.cpp:503-533`.
All follow the same pattern: check error → pick severity color → show/hide →
start/stop pulse.

### F. Slot Bar Column

```cpp
struct SlotColumn {
    lv_obj_t* container;    // Column flex wrapper
    lv_obj_t* bar_bg;       // Background/outline
    lv_obj_t* bar_fill;     // Colored fill
    lv_obj_t* status_line;  // Bottom indicator
};

struct BarStyleParams {
    uint32_t color_rgb = 0x808080;
    int fill_pct = 100;
    bool is_present = false;
    bool is_loaded = false;
    bool has_error = false;
    SlotError::Severity severity = SlotError::INFO;
};

constexpr int32_t STATUS_LINE_HEIGHT_PX = 3;
constexpr int32_t STATUS_LINE_GAP_PX = 2;

SlotColumn create_slot_column(lv_obj_t* parent, int32_t bar_width,
                              int32_t bar_height, int32_t bar_radius);
void style_slot_bar(const SlotColumn& col, const BarStyleParams& params,
                    int32_t bar_radius);
```

**Replaces:** `ui_ams_mini_status.cpp:192-228` (create_slot_container) +
`:126-183` (update_slot_bar) and `ui_panel_ams_overview.cpp:549-631`
(inline bar creation in create_mini_bars).

The `style_slot_bar` function applies the overview card's visual style:
- **Loaded:** 2px border, `text` color, 80% opacity
- **Present:** 1px border, `text_muted`, 50% opacity
- **Empty:** 1px border, `text_muted`, 20% opacity (ghosted)
- **Error status line:** severity-aware (danger/warning)
- **Non-error:** status line hidden (loaded shown via border)
- **Fill gradient:** lighten_color(base, 50) top → base bottom

### G. Bar Width Calculation

```cpp
int32_t calc_bar_width(int32_t container_width, int slot_count,
                       int32_t gap, int32_t min_width, int32_t max_width,
                       int container_pct = 100);
```

**Replaces:** 3 copies of the same math in `ui_ams_mini_status.cpp:317-320,
390-393` and `ui_panel_ams_overview.cpp:531-538`.

### H. Fill Percentage

```cpp
int fill_percent_from_slot(const SlotInfo& slot, int min_pct = 5);
```

**Replaces:** Manual calculation in `ui_ams_mini_status.cpp:688-693` and
`ui_panel_ams_overview.cpp:604-606`.

### I. Transparent Container Factory

```cpp
lv_obj_t* create_transparent_container(lv_obj_t* parent);
```

Applies: no bg, no border, no padding, no scroll, event bubble. Replaces
7-line boilerplate that appears 6+ times across mini_status and overview.

### J. Logo with Fallback

```cpp
void apply_logo(lv_obj_t* image, const AmsUnit& unit,
                const AmsSystemInfo& info);
void apply_logo(lv_obj_t* image, const std::string& type_name);
```

Try unit name → type name → hide. **Replaces:** 4 copies of the fallback
chain in `ui_panel_ams_overview.cpp:416-427, 927-938` and
`ui_panel_ams.cpp:528-539, 553-562`.

### K. Unit Display Name

```cpp
std::string get_unit_display_name(const AmsUnit& unit, int unit_index);
```

**Replaces:** `ui_panel_ams_overview.cpp:141-146` and inline reimplementation
at `ui_panel_ams.cpp:545`.

## Changes to `ams_types.h`

### L. `SlotInfo::is_present()`

```cpp
[[nodiscard]] bool is_present() const {
    return status != SlotStatus::EMPTY && status != SlotStatus::UNKNOWN;
}
```

**Replaces:** 3 different "is present" expressions in mini_status, overview,
and ams_slot.

## Cleanup (No New Shared Code)

### M. Use `theme_manager_get_contrast_text()`

`ui_ams_slot.cpp:274-275` and `:403-404` hand-roll
`brightness > 140 ? black : white`. Replace with existing
`theme_manager_get_contrast_text(bg_color)`.

## Files Modified

| File | Changes |
|------|---------|
| **NEW** `src/ui/ams_drawing_utils.h` | All shared declarations |
| **NEW** `src/ui/ams_drawing_utils.cpp` | All shared implementations |
| `include/ams_types.h` | Add `SlotInfo::is_present()` |
| `src/ui/ui_ams_mini_status.cpp` | Use shared utils, adopt overview visual style |
| `src/ui/ui_panel_ams_overview.cpp` | Use shared utils, remove local helpers |
| `src/ui/ui_ams_slot.cpp` | Use shared pulse/color, fix contrast text |
| `src/ui/ui_spool_canvas.cpp` | Use shared color utils |
| `src/ui/ui_panel_ams.cpp` | Use shared logo/display-name helpers |

## What Stays Per-File

- **Mini status:** Multi-unit row layout, observer binding, XML widget
  registration, overflow label, stacked-rows container management
- **Overview panel:** Unit card creation/update, detail view zoom animations,
  system path canvas, context menu, detail slot management
- **Slot widget:** Spool canvas integration, loaded-slot highlight (3px
  border + shadow glow on spool_container), tool badge, label management
- **Spool canvas:** 3D spool drawing primitives (gradient ellipse, rect,
  edge highlights) — specialized, not shared
- **AMS panel:** Backend selector, slot grid via slot_layout module, scoped
  unit navigation

## Estimated Impact

~350 lines of duplicated code removed across 6 files, replaced by ~150 lines
in shared utils. Net reduction ~200 lines.
