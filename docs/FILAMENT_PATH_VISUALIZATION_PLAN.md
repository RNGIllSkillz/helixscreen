# Filament Path Visualization for AMS Panel

## Summary

Add schematic filament path visualization to the AMS panel showing routing from spools through hub/selector to extruder. Support both Happy Hare (linear) and AFC (hub) topologies with active path highlighting, load/unload animations, and segment-based error inference.

---

## Prerequisites

**Pull existing AMS implementation from blackmac.local:**
```bash
scp -r pbrown@blackmac.local:Code/Printing/helixscreen-ams-feature/include/ams* /Users/pbrown/code/helixscreen-ams/include/
scp -r pbrown@blackmac.local:Code/Printing/helixscreen-ams-feature/src/ams* /Users/pbrown/code/helixscreen-ams/src/
scp -r pbrown@blackmac.local:Code/Printing/helixscreen-ams-feature/ui_xml/ams* /Users/pbrown/code/helixscreen-ams/ui_xml/
scp -r pbrown@blackmac.local:Code/Printing/helixscreen-ams-feature/src/ui_spool* /Users/pbrown/code/helixscreen-ams/src/
scp -r pbrown@blackmac.local:Code/Printing/helixscreen-ams-feature/include/ui_spool* /Users/pbrown/code/helixscreen-ams/include/
scp -r pbrown@blackmac.local:Code/Printing/helixscreen-ams-feature/src/ui_ams* /Users/pbrown/code/helixscreen-ams/src/
scp -r pbrown@blackmac.local:Code/Printing/helixscreen-ams-feature/include/ui_ams* /Users/pbrown/code/helixscreen-ams/include/
```

Then verify build: `cd /Users/pbrown/code/helixscreen-ams && make -j`

---

## Design Decisions

| Decision | Choice |
|----------|--------|
| **Topology** | Support both: Linear (Happy Hare) and Hub (AFC) - auto-detect |
| **Visual style** | Schematic/diagram - clean lines like circuit diagram |
| **Error display** | Segment-based inference from sensor/state data |
| **Visibility** | Always visible as part of main AMS panel |
| **Implementation** | Custom LVGL canvas widget following `ui_jog_pad.cpp` pattern |

---

## Architecture

### Layout (Vertical, Portrait-Optimized)

**Key principle**: Spools at top, flowing down through hub/selector to extruder at bottom. Spools are integrated as tappable nodes in the path diagram itself.

```
┌─────────────────────────────────────────┐
│  Header: "Multi-Filament"               │
├─────────────────────────────────────────┤
│                                         │
│   ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐   │  ← Spool row (tappable)
│   │ ◐ 0  │ │ ◐ 1  │ │ ◐ 2  │ │ ◐ 3  │   │
│   └──┬───┘ └──┬───┘ └──┬───┘ └──┬───┘   │
│      │        │        │        │       │  ← Lane/gate lines
│      ○        ○        ○        ○       │  ← Prep sensors (AFC)
│      │        │        │        │       │
│      └────────┴────┬───┴────────┘       │
│                    │                    │  ← Merge point
│               ┌────┴────┐               │
│               │   HUB   │               │  ← Hub/Selector
│               └────┬────┘               │
│                    ○                    │  ← Hub sensor
│                    │                    │
│                    │                    │  ← Output tube
│                    ○                    │  ← Toolhead sensor
│                    │                    │
│               ┌────┴────┐               │
│               │ EXTRUDER│               │  ← Nozzle
│               └─────────┘               │
│                                         │
├─────────────────────────────────────────┤
│  Status: Idle            [Unload][Home] │
└─────────────────────────────────────────┘

◐ = Spool with filament color fill
○ = Sensor point
```

### Hub Topology (AFC) - Vertical
```
    [0]     [1]     [2]     [3]      ← Spools at top (tappable nodes)
     │       │       │       │
     ○       ○       ○       ○       ← Prep sensors
     │       │       │       │
     └───────┴───┬───┴───────┘       ← Lanes merge
                 │
            ┌────┴────┐
            │   HUB   │              ← Hub/merger
            └────┬────┘
                 ○                   ← Hub sensor
                 │
                 ○                   ← Toolhead sensor
                 │
            ┌────┴────┐
            │▼ NOZZLE │              ← Extruder
            └─────────┘
```

### Linear Topology (Happy Hare) - Vertical
```
    [0]     [1]     [2]     [3]      ← Spools at top
     │       │       │       │
     └───────┴───┬───┴───────┘       ← Gates to selector
                 │
            ┌────┴────┐
            │SELECTOR │              ← Selector mechanism
            └────┬────┘
                 ║
                 ║                   ← Bowden tube (double line)
                 ║
                 ○                   ← Toolhead sensor
                 │
            ┌────┴────┐
            │▼ NOZZLE │              ← Extruder
            └─────────┘
```

### Visual States
| State | Rendering |
|-------|-----------|
| **Idle lane** | Thin gray dashed line |
| **Available** | Thin gray solid line |
| **Active/loaded** | Thick line in filament color |
| **Loading** | Animated gradient moving downward |
| **Unloading** | Animated gradient moving upward |
| **Error segment** | Thick red pulsing line |
| **Active spool** | Highlighted border + glow |

---

## Data Model

### Unified Path Model

**Philosophy**: Happy Hare and AFC use different terminology but describe the same physical routing stages. We use AFC-inspired naming with a unified "hub" concept that represents either a selector (HH) or merger (AFC).

```
┌─────────────────────────────────────────────────────────────────────────┐
│                     UNIFIED FILAMENT PATH MODEL                         │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   SPOOL ──── PREP ──── LANE ──── HUB ──── OUTPUT ──── TOOLHEAD ──── NOZZLE
│     │         │         │         │          │           │           │
│   Storage   Entry    Routing   Router     Final       Entry        Loaded
│            sensor    segment   point      tube       sensor
│                                                                         │
├─────────────────────────────────────────────────────────────────────────┤
│  Happy Hare mapping:                                                    │
│   SPOOL=Gate, PREP=Gate sensor, LANE=Gate-to-selector,                  │
│   HUB=Selector, OUTPUT=Bowden, TOOLHEAD=Extruder sensor                 │
├─────────────────────────────────────────────────────────────────────────┤
│  AFC mapping:                                                           │
│   SPOOL=Lane spool, PREP=Prep sensor, LANE=Lane tube,                   │
│   HUB=Hub/Merger, OUTPUT=Output tube, TOOLHEAD=Toolhead sensor          │
└─────────────────────────────────────────────────────────────────────────┘
```

### New Enums (`ams_types.h`)

```cpp
/**
 * @brief Path topology - affects visual rendering only, not segment model
 */
enum class PathTopology {
    LINEAR = 0,  // Happy Hare: selector picks one input
    HUB = 1      // AFC: merger combines inputs
};

/**
 * @brief Unified path segments (AFC-inspired naming)
 *
 * Both Happy Hare and AFC map to these same logical segments.
 * The widget draws them differently based on PathTopology.
 */
enum class PathSegment {
    NONE = 0,        // No segment / idle
    SPOOL = 1,       // At spool (filament storage)
    PREP = 2,        // At entry sensor (prep/gate sensor)
    LANE = 3,        // In lane/gate-to-router segment
    HUB = 4,         // At router (selector or hub/merger)
    OUTPUT = 5,      // In output tube (bowden or hub output)
    TOOLHEAD = 6,    // At toolhead sensor
    NOZZLE = 7       // Fully loaded in nozzle
};

// Convenience: number of segments for iteration
constexpr int PATH_SEGMENT_COUNT = 7;
```

### Mapping Functions

```cpp
// In ams_backend_happy_hare.cpp
PathSegment map_filament_pos_to_segment(int filament_pos) {
    // HH filament_pos: 0=unloaded, 1=homed_gate, 2=in_gate, 3=in_bowden,
    //                  4=end_bowden, 5=homed_extruder, 6=extruder_entry,
    //                  7=in_extruder, 8=loaded
    switch (filament_pos) {
        case 0: return PathSegment::SPOOL;
        case 1:
        case 2: return PathSegment::PREP;     // Gate area
        case 3: return PathSegment::LANE;     // Moving through
        case 4: return PathSegment::HUB;      // At selector
        case 5: return PathSegment::OUTPUT;   // In bowden
        case 6: return PathSegment::TOOLHEAD; // At extruder
        case 7:
        case 8: return PathSegment::NOZZLE;   // Loaded
        default: return PathSegment::NONE;
    }
}

// In ams_backend_afc.cpp
PathSegment infer_segment_from_sensors(bool prep, bool hub, bool toolhead) {
    if (toolhead) return PathSegment::NOZZLE;
    if (hub) return PathSegment::TOOLHEAD;  // Past hub, approaching toolhead
    if (prep) return PathSegment::HUB;       // Past prep, approaching hub
    return PathSegment::SPOOL;               // Not yet at prep
}
```

### New Subjects (`AmsState`)

| Subject | Type | Description |
|---------|------|-------------|
| `ams_path_topology` | int | PathTopology enum (visual only) |
| `ams_path_active_gate` | int | Currently loaded gate (-1=none) |
| `ams_path_filament_segment` | int | PathSegment where filament currently is |
| `ams_path_error_segment` | int | PathSegment with error (for highlighting) |
| `ams_path_anim_progress` | int | Animation progress 0-100 |

---

## Error Inference Logic

Both systems use the **same unified PathSegment enum**. The backend translates system-specific state to the common model.

### Happy Hare → PathSegment
```
printer.mmu.filament_pos → PathSegment
  0 (unloaded)         → SPOOL
  1-2 (gate area)      → PREP
  3 (in bowden)        → LANE
  4 (end bowden)       → HUB
  5 (homed extruder)   → OUTPUT
  6 (extruder entry)   → TOOLHEAD
  7-8 (loaded)         → NOZZLE
```

### AFC → PathSegment
```
Sensor state → PathSegment
  No sensors           → SPOOL (runout at source)
  Prep only            → LANE (between prep and hub)
  Prep + Hub           → OUTPUT (between hub and toolhead)
  All three            → NOZZLE (fully loaded)

Error location = last sensor that was OK
```

### Unified Error Display
The path canvas widget receives `ams_path_error_segment` (a PathSegment value) and highlights that segment in error color regardless of which backend set it.

---

## Implementation Phases

### Phase 1: Setup & Integration (30 min)
1. Pull existing AMS code from blackmac.local
2. Verify build passes
3. Test `./build/bin/helix-screen --test -p ams -vv`
4. Commit pulled files

### Phase 2: Data Model Extensions (1 hr)
1. Add `PathTopology`, `PathSegment` enums to `ams_types.h`
2. Add path subjects to `AmsState`
3. Add `get_topology()`, `infer_error_segment()` to `AmsBackend` interface
4. Implement in `AmsBackendMock`

### Phase 3: Path Canvas Widget (2-3 hrs)
**Files to create:**
- `include/ui_filament_path_canvas.h`
- `src/ui_filament_path_canvas.cpp`

**Implementation:**
1. Register as LVGL XML widget (follow `ui_jog_pad.cpp` pattern)
2. Store state in widget user_data struct
3. Draw via `LV_EVENT_DRAW_POST` callback
4. Handle tap events for spool selection
5. Support XML attributes: `topology`, `gate_count`, `active_gate`

**Vertical Layout Drawing (top to bottom):**
1. **Spool row** (top): Horizontal row of spool icons with filament colors
   - Use existing `spool_canvas` or simplified circles
   - Tappable - `LV_EVENT_CLICKED` triggers gate selection
2. **Lane lines**: Vertical lines from each spool down
3. **Prep sensors** (AFC): Small circles on lane lines
4. **Merge point**: Lines curve inward to center
5. **Hub/Selector**: Rounded rectangle in center
6. **Hub sensor**: Small circle below hub
7. **Output tube**: Single/double vertical line down
8. **Toolhead sensor**: Small circle near bottom
9. **Extruder/Nozzle**: Nozzle icon at bottom

**Drawing primitives:**
- `lv_draw_line()` for path segments (with rounded caps)
- `lv_draw_arc()` for curved merge paths
- `lv_draw_rect()` for hub/selector/extruder boxes
- `lv_draw_arc()` or custom for spool circles
- Theme colors from `ui_theme_parse_color()`

### Phase 4: State Binding (1 hr)
1. Add observers in `ui_panel_ams.cpp` for path subjects
2. Call `ui_filament_path_canvas_set_*()` APIs on state changes
3. Invalidate canvas on `ams_path_version` change

### Phase 5: Animation (1-2 hrs)
1. Use LVGL `lv_anim_t` for progress animation
2. Draw animated "filament segment" moving along path during load/unload
3. Pulse animation for error segments

### Phase 6: Real Backend Integration (1-2 hrs)
1. Parse `printer.mmu.filament_pos` in Happy Hare backend
2. Parse AFC sensor states
3. Implement `infer_error_segment()` in real backends

### Phase 7: Polish (1 hr)
1. Responsive sizing for different screens
2. Touch feedback (tap segment to see info?)
3. Update `AMS_IMPLEMENTATION_PLAN.md`

---

## Files to Create

| File | Purpose |
|------|---------|
| `include/ui_filament_path_canvas.h` | Path canvas widget header |
| `src/ui_filament_path_canvas.cpp` | Path canvas widget implementation |

## Files to Modify

| File | Changes |
|------|---------|
| `include/ams_types.h` | Add PathTopology, PathSegment enums |
| `include/ams_state.h` | Add path subjects |
| `src/ams_state.cpp` | Initialize path subjects |
| `include/ams_backend.h` | Add get_topology(), infer_error_segment() |
| `src/ams_backend_mock.cpp` | Implement path methods |
| `src/ams_backend_happy_hare.cpp` | Parse filament_pos, implement inference |
| `src/ams_backend_afc.cpp` | Parse sensors, implement inference |
| `ui_xml/ams_panel.xml` | Replace slot grid with integrated path canvas (full width, vertical) |
| `src/ui_panel_ams.cpp` | Add path state observers |
| `src/main.cpp` | Register ui_filament_path_canvas widget |

---

## Reference Patterns

- **Custom canvas widget**: `src/ui_jog_pad.cpp` - draw callback, state struct, theme colors
- **Animation**: `src/ui_heating_animator.cpp` - LVGL lv_anim_t usage
- **Gradient drawing**: `src/ui_gradient_canvas.cpp` - canvas buffer management

---

## Testing

```bash
# Build
cd /Users/pbrown/code/helixscreen-ams && make -j

# Run with mock AMS
./build/bin/helix-screen --test -p ams -s large -vv

# Screenshot
HELIX_AUTO_SCREENSHOT=1 HELIX_AUTO_QUIT_MS=3000 ./build/bin/helix-screen --test -p ams -vv
```

---

## Estimated Time

| Phase | Time |
|-------|------|
| Setup & pull code | 30 min |
| Data model | 1 hr |
| Path canvas widget | 2-3 hrs |
| State binding | 1 hr |
| Animation | 1-2 hrs |
| Backend integration | 1-2 hrs |
| Polish | 1 hr |
| **Total** | **7-10 hrs** |
