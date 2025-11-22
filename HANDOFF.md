# Session Handoff Document

**Last Updated:** 2025-11-21
**Current Focus:** G-code viewer performance optimizations complete

---

## ✅ CURRENT STATE

### Recently Completed

**G-Code Viewer Performance & Multicolor Rendering (2025-11-21)**
- ✅ **Phase 1 - Performance optimizations + multicolor rendering fixes:**
  - Fixed multicolor rendering bug (OrcaCube_ABS_Multicolor.gcode now displays properly)
  - Configured tool color palette in async geometry builder
  - Added multicolor detection to prevent color override (get_geometry_color_count())
  - Added callback-based async load completion API (gcode_viewer_load_callback_t)
  - Implemented ui_gcode_viewer_get_filename() API
  - Enhanced file info display with filename and simplified filament type
  - Fixed XML long_mode="wrap" syntax
  - Added FPS tracking for performance monitoring (debug level)
- ✅ **Phase 2 - Vertex sharing diagnostics and analysis:**
  - Added vertex sharing statistics tracking
  - Discovered 92.4% vertex sharing rate (34,496/37,347 segments)
  - Original estimate of ~3% was incorrect - vertex sharing already highly optimized
  - Analyzed framebuffer RGB conversion (already optimal)
  - Concluded no further optimization needed

**Performance Metrics (OrcaCube_ABS_Multicolor.gcode):**
- Geometry build: 0.056s
- First render: 99.2ms
- Memory: 10.85 MB
- Vertex sharing: 92.4% (34,496/37,347 segments)
- Normal palette: 10k entries (down from 44k via hash map optimization)

**Commits:**
- `ebf361e` - perf(gcode): Phase 2 - vertex sharing diagnostics and analysis
- `8102c34` - perf(gcode): Phase 1 performance optimizations + multicolor rendering fixes

**N-Sided Elliptical Tube Rendering (2025-11-21)**
- ✅ Implemented configurable N-sided tube cross-sections (N=4, 8, or 16)
- ✅ Elliptical cross-section (width ≠ height) matching FDM extrusion geometry
- ✅ Phase 1: Infrastructure and config (tube_sides parameter)
- ✅ Phase 2: Data structures (vectors instead of hardcoded arrays)
- ✅ Phase 3: Vertex generation (N-based loops with elliptical positioning)
- ✅ Phase 4: Triangle strip generation (N-based with triangle fans for caps)
- ✅ Fixed face winding order for correct backface culling
- ✅ Performance: Hash map optimization for palette lookups (146× speedup)
- ✅ Performance: Re-enabled segment simplification (54% reduction)
- ✅ Cleaned up unused variables and obsolete code
- ✅ Default: N=4 (diamond, fastest) for best performance
- ✅ Optional: N=8 (octagonal), N=16 (circular, matches OrcaSlicer quality)

**Configuration:**
```json
{
  "gcode_viewer": {
    "tube_sides": 4  // Options: 4 (default), 8, 16
  }
}
```

**Commits:**
- `3bf1c42` - docs: archive GCODE_TUBE_NSIDED_REFACTOR.md
- `428b471` - config: change default gcode-test panel file to OrcaCube AD5M
- `d945752` - config: change default tube_sides from 16 to 4
- `26e2a33` - fix(gcode): correct face winding order for N-sided tubes
- `e4d9088` - refactor: clean up unused variables and obsolete code
- `774344b` - feat(gcode): Enable N=8 and N=16 elliptical tube cross-sections
- `62ba4b5` - wip(gcode): Phase 4 - Triangle strip generation for N-sided tubes
- `b3d2368` - wip(gcode): Phase 3 - Vertex generation refactor for N-sided tubes
- `0ab2f42` - wip(gcode): Phase 2 - Data structure refactor for N-sided tubes

**Async G-Code Rendering (2025-11-20/21)**
- ✅ Non-blocking geometry building in background thread
- ✅ UI remains responsive during large file loads
- ✅ Callback-based completion notification
- ✅ File info panel updates automatically after load

---

## 🚀 NEXT PRIORITIES

### 1. **Complete Bed Mesh UI Features** (HIGH PRIORITY)

**Goal:** Match feature parity with GuppyScreen bed mesh visualization

**Completed:**
- ✅ Rotation sliders (Tilt/Spin) - visible and functional
- ✅ Info labels (dimensions, Z range) - visible with reactive bindings
- ✅ Grid lines - wireframe overlay properly aligned

**Remaining Tasks:**
1. [ ] Add axis labels (X/Y/Z indicators, bed dimensions)
2. [ ] Add mesh profile selector dropdown
3. [ ] Display additional statistics (variance/deviation)

### 2. **G-Code Viewer - Layer Controls** (MEDIUM PRIORITY)

**Remaining Tasks:**
- [ ] Add layer slider/scrubber for preview-by-layer
- [ ] Add layer range selector (start/end layer)
- [ ] Show current layer number in UI
- [ ] Test with complex multi-color G-code files

**Command-Line Testing:**
```bash
# Test with custom camera angles
./build/bin/helix-ui-proto -p gcode-test --test --gcode-az 90 --gcode-el -10 --gcode-zoom 5

# Test with multi-color file and debug colors
./build/bin/helix-ui-proto -p gcode-test --test \
  --gcode-file assets/test_gcode/multi_color_cube.gcode \
  --gcode-debug-colors

# Test specific camera position for screenshots
./build/bin/helix-ui-proto -p gcode-test --test \
  --gcode-file my_print.gcode \
  --gcode-az 45 --gcode-el 30 --gcode-zoom 2.0 \
  --screenshot 2
```

### 3. **Print Select Integration**

**Goal:** Add "Preview" button to print file browser

**Tasks:**
- [ ] Add preview button to file list items in print_select_panel.xml
- [ ] Fetch G-code via Moonraker HTTP API
- [ ] Open viewer in overlay panel
- [ ] Show filename and stats (layers, print time)

---

## 📋 Critical Patterns Reference

### Pattern #0: Per-Face Debug Coloring (NEW)

**Purpose:** Diagnose 3D geometry issues by coloring each face differently

**Usage:**
```cpp
// Enable in renderer constructor or via method
geometry_builder_->set_debug_face_colors(true);

// Colors assigned:
// - Top face: Red (#FF0000)
// - Bottom face: Blue (#0000FF)
// - Left face: Green (#00FF00)
// - Right face: Yellow (#FFFF00)
// - Start cap: Magenta (#FF00FF)
// - End cap: Cyan (#00FFFF)
```

**Implementation:**
- Creates 6 color palette entries when enabled
- Assigns colors based on vertex face membership
- Vertex order: [0-1]=bottom, [2-3]=right, [4-5]=top, [6-7]=left
- Logs once per build: "DEBUG FACE COLORS ACTIVE: ..."

**When to Use:**
- Diagnosing twisted/kinked geometry
- Verifying face orientation and winding order
- Checking vertex ordering correctness
- Validating normal calculations

### Pattern #1: G-Code Camera Angles

**Exact test angle for geometry verification:**
```cpp
// In gcode_camera.cpp reset():
azimuth_ = 85.5f;    // Horizontal rotation
elevation_ = -2.5f;  // Slight downward tilt
zoom_level_ = 10.0f; // 10x zoom (preserved by fit_to_bounds)
```

**Debug Overlay (requires -vv flag):**
- Shows "Az: 85.5° El: -2.5° Zoom: 10.0x" in upper-left
- Only visible with debug logging enabled
- Helps reproduce exact viewing conditions

### Pattern #2: LV_SIZE_CONTENT Bug

**NEVER use `height="LV_SIZE_CONTENT"` or `height="auto"` with complex nested children in flex layouts.**

```xml
<!-- ❌ WRONG - collapses to 0 height -->
<ui_card height="LV_SIZE_CONTENT" flex_flow="column">
  <text_heading>...</text_heading>
  <lv_dropdown>...</lv_dropdown>
</ui_card>

<!-- ✅ CORRECT - uses flex grow -->
<ui_card flex_grow="1" flex_flow="column">
  <text_heading>...</text_heading>
  <lv_dropdown>...</lv_dropdown>
</ui_card>
```

### Pattern #3: Bed Mesh Widget API

**Custom LVGL widget for bed mesh canvas:**

```cpp
#include "ui_bed_mesh.h"

// Set mesh data (triggers automatic redraw)
std::vector<const float*> row_pointers;
// ... populate row_pointers ...
ui_bed_mesh_set_data(canvas, row_pointers.data(), rows, cols);

// Update rotation (triggers automatic redraw)
ui_bed_mesh_set_rotation(canvas, tilt_angle, spin_angle);

// Force redraw
ui_bed_mesh_redraw(canvas);
```

**Widget automatically manages:**
- Canvas buffer allocation (600×400 RGB888 = 720KB)
- Renderer lifecycle (create on init, destroy on delete)
- Layout updates (calls lv_obj_update_layout before render)
- Bounds checking (clips all coordinates to canvas)

### Pattern #4: Reactive Subjects for Mesh Data

```cpp
// Initialize subjects
static lv_subject_t bed_mesh_dimensions;
static char dimensions_buf[64] = "No mesh data";
lv_subject_init_string(&bed_mesh_dimensions, dimensions_buf,
                       prev_buf, sizeof(dimensions_buf), "No mesh data");

// Update when mesh changes
snprintf(dimensions_buf, sizeof(dimensions_buf), "%dx%d points", rows, cols);
lv_subject_copy_string(&bed_mesh_dimensions, dimensions_buf);
```

```xml
<!-- Bind label to subject -->
<lv_label name="mesh_dimensions_label" bind_text="bed_mesh_dimensions"/>
```

### Pattern #5: Thread Management - NEVER Block UI Thread

**CRITICAL:** NEVER use blocking operations like `thread.join()` in code paths triggered by UI events.

```cpp
// ❌ WRONG - Blocks LVGL main thread
if (connect_thread_.joinable()) {
    connect_thread_.join();  // UI FREEZES HERE
}

// ✅ CORRECT - Non-blocking cleanup
connect_active_ = false;
if (connect_thread_.joinable()) {
    connect_thread_.detach();
}
```

### Pattern #6: G-code Viewer Widget API

**Custom LVGL widget for G-code 3D visualization:**

```cpp
#include "ui_gcode_viewer.h"

// Create viewer widget
lv_obj_t* viewer = ui_gcode_viewer_create(parent);

// Load G-code file
ui_gcode_viewer_load_file(viewer, "/path/to/file.gcode");

// Change view
ui_gcode_viewer_set_view(viewer, GCODE_VIEW_ISOMETRIC);
ui_gcode_viewer_set_view(viewer, GCODE_VIEW_TOP);

// Reset camera
ui_gcode_viewer_reset_view(viewer);
```

```xml
<!-- Use in XML -->
<gcode_viewer name="my_viewer" width="100%" height="400"/>
```

**Widget features:**
- Touch drag to rotate camera
- Automatic fit-to-bounds framing
- Preset view buttons
- State management (EMPTY/LOADING/LOADED/ERROR)

---

## 📚 Key Documentation

- **G-code Visualization:** `docs/GCODE_VISUALIZATION.md` - Complete system design and integration guide
- **Bed Mesh Analysis:** `docs/GUPPYSCREEN_BEDMESH_ANALYSIS.md` - GuppyScreen renderer analysis
- **Implementation Patterns:** `docs/BEDMESH_IMPLEMENTATION_PATTERNS.md` - Code templates
- **Renderer API:** `docs/BEDMESH_RENDERER_INDEX.md` - bed_mesh_renderer.h reference
- **Widget APIs:** `include/ui_bed_mesh.h`, `include/ui_gcode_viewer.h` - Custom widget public APIs

---

## 🐛 Known Issues

1. **Missing Bed Mesh UI Features**
   - ✅ Grid lines - IMPLEMENTED
   - ✅ Info labels - IMPLEMENTED
   - ✅ Rotation sliders - IMPLEMENTED
   - ❌ Axis labels not implemented
   - ❌ Mesh profile selector not implemented
   - ❌ Variance/deviation statistics not displayed

2. **No Bed Mesh Profile Switching**
   - Can fetch multiple profiles from Moonraker
   - No UI to switch between profiles

3. **G-code Viewer Not Integrated**
   - Standalone test panel works (`-p gcode-test`)
   - Not yet integrated with print select panel
   - No "Preview" button in file browser

---

## 🔍 Debugging Tips

**G-Code Viewer Testing:**
```bash
# Run with debug logging and camera overlay
./build/bin/helix-ui-proto -p gcode-test -vv --test

# Test multi-color files
./build/bin/helix-ui-proto -p gcode-test --test-file assets/test_gcode/multi_color_cube.gcode

# Camera overlay shows (upper-left, only with -vv):
# "Az: 85.5° El: -2.5° Zoom: 10.0x"
```

**Screenshot Current State:**
```bash
# Take screenshot
./scripts/screenshot.sh helix-ui-proto gcode-test gcode-test

# View screenshot
open /tmp/ui-screenshot-gcode-test.png
```

**Per-Face Debug Coloring (if needed):**
```cpp
// Enable in gcode_tinygl_renderer.cpp:31
geometry_builder_->set_debug_face_colors(true);

// Colors: Top=Red, Bottom=Blue, Left=Green, Right=Yellow,
//         StartCap=Magenta, EndCap=Cyan
```
