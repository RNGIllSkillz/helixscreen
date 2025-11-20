# Session Handoff Document

**Last Updated:** 2025-11-20
**Current Focus:** G-code TinyGL Renderer - Tube Geometry Debugging

---

## ✅ CURRENT STATE

### Just Completed (This Session) - 2025-11-20

**G-Code TinyGL Renderer: Per-Face Debug Coloring System**

**Context:** Discovered critical geometry issues in 3D rectangular tube rendering. Tubes appearing twisted/kinked instead of straight, with triangular gaps and incorrect face positioning.

**Implementation:** Added comprehensive per-face debug coloring system to diagnose geometry problems:

**Files Modified:**
1. `include/gcode_geometry_builder.h` (lines 278-295, 317)
   - Added `bool debug_face_colors_` member variable
   - Added `set_debug_face_colors(bool enable)` method with full documentation
   - Documents 6-color mapping: Top=Red, Bottom=Blue, Left=Green, Right=Yellow, StartCap=Magenta, EndCap=Cyan

2. `src/gcode_geometry_builder.cpp` (lines 18-29, 558-573, 586-625, 628-661, 681-691, 702-708)
   - Added `DebugColors` namespace with 6 bright color constants
   - Modified color assignment in `generate_ribbon_vertices()`:
     - Creates 6 separate color palette indices when debug mode enabled
     - Assigns face-specific colors to all vertices based on which face they belong to
     - Vertex ordering: [0-1]=bottom (Blue), [2-3]=right (Yellow), [4-5]=top (Red), [6-7]=left (Green)
     - Start cap vertices use Magenta, end cap vertices use Cyan
   - Added debug logging (once per build) showing active color mapping

3. `include/gcode_tinygl_renderer.h` (lines 98-110)
   - Added `set_debug_face_colors(bool enable)` public method
   - Complete Doxygen documentation with 6-color mapping

4. `src/gcode_tinygl_renderer.cpp` (lines 31, 91-101)
   - Enabled debug mode in constructor: `geometry_builder_->set_debug_face_colors(true)` (line 31)
   - Implemented `set_debug_face_colors()` wrapper method with debug logging
   - Method triggers geometry rebuild to apply new coloring

5. `src/gcode_camera.cpp` (lines 28-38)
   - Set camera to exact test angle: Az=85.5°, El=-2.5°, Zoom=10.0x
   - Preserves custom zoom through `fit_to_bounds()` calls

**Debug Output (with -vv flag):**
```
[debug] Setting debug face colors: ENABLED
[debug] DEBUG FACE COLORS ACTIVE: Top=Red, Bottom=Blue, Left=Green,
        Right=Yellow, StartCap=Magenta, EndCap=Cyan
```

**🚨 CRITICAL DISCOVERY: Major Geometry Issues Revealed**

The per-face coloring successfully exposed fundamental problems in tube geometry generation:

**Observable Issues (from screenshot `/tmp/ui-screenshot-gcode-debug-colors.png`):**
1. ✅ **Debug colors working perfectly** - Can clearly see all 6 faces in distinct colors
2. ❌ **Tube is twisted/kinked** - 10mm horizontal line renders as bent/curved shape
3. ❌ **Black triangular gaps** - Missing or incorrectly connected geometry
4. ❌ **Wrong overall shape** - Should be straight rectangular bar, appears twisted

**Color Verification:**
- Cyan (end cap) - visible at left front ✓
- Blue (bottom) - visible on bottom face ✓
- Yellow (right) - visible on right side ✓
- Magenta (start cap) - visible at right end ✓
- Geometry is clearly WRONG despite colors being correct

**Root Cause Analysis (Suspected):**

The issue is in `generate_ribbon_vertices()` (gcode_geometry_builder.cpp:451-720). Possible causes:

1. **Vertex Ordering Issue:**
   - TubeCap order: `[bl_bottom, br_bottom, br_right, tr_right, tr_top, tl_top, tl_left, bl_left]`
   - Each vertex appears TWICE with different normals (8 vertices per cross-section)
   - Start/end cap vertices might be misaligned causing twist

2. **Triangle Strip Winding:**
   - Face strips connect start→end vertices in strips
   - If vertex order differs between start/end caps, causes twist
   - Lines 697-708 define strips: bottom, right, top, left faces

3. **Cross-Section Calculation:**
   - Perpendicular vectors computed from direction (lines 473-485)
   - `perp_horizontal = cross(direction, up)`
   - `perp_vertical = cross(direction, perp_horizontal)`
   - May need verification for horizontal lines

**Test File:** `assets/single_line_test.gcode`
```gcode
G1 X0 Y0 F7800
G1 X10 Y0 E1.0 F1800 ; single 10mm extrusion from (0,0,0.2) to (10,0,0.2)
```

**Expected Result:** Straight rectangular tube from (0,0,0.2) to (10,0,0.2)
- Dimensions: 10mm × 0.42mm × 0.2mm (length × width × height)
- Should appear perfectly straight when viewed from Az=85.5°, El=-2.5°

**Actual Result:** Twisted/kinked tube with gaps

**Next Steps (HIGH PRIORITY):**

1. **Investigate Vertex Generation Logic:**
   - Examine `generate_ribbon_vertices()` vertex ordering
   - Verify start/end cap vertices match in position and order
   - Check if vertex reuse (`prev_start_cap`) introduces misalignment

2. **Verify Triangle Strip Construction:**
   - Lines 697-708: bottom/right/top/left face strips
   - Ensure strips connect corresponding vertices correctly
   - Check winding order (CCW vs CW)

3. **Cross-Section Geometry Validation:**
   - Add debug logging for vertex positions
   - Verify perpendicular vectors for horizontal direction
   - Check if normal calculations affect vertex positioning

4. **Consider Simplification:**
   - Current approach: 8 vertices per cross-section (duplicate corners for different face normals)
   - May need to separate vertex positions from normals
   - Could use indexed geometry with unique positions + per-vertex normals

**Files to Focus On:**
- `src/gcode_geometry_builder.cpp:451-720` - `generate_ribbon_vertices()`
- `src/gcode_geometry_builder.cpp:697-715` - Triangle strip construction
- Review vertex ordering and strip winding documentation

**Commits:**
- (Pending) - feat(gcode): add per-face debug coloring for geometry diagnosis
- (Pending) - docs: document discovered tube geometry issues

---

### Previously Completed (Earlier This Session)

**G-Code Rendering: Layer Height and Normal Fixes**
- ✅ Fixed tube proportions using `layer_height_mm` from G-code (was using width*0.25)
- ✅ Corrected face normals to point outward (top=UP, bottom=DOWN)
- ✅ Added camera debug overlay showing Az/El/Zoom (only with -vv flag)
- ✅ Set layer height from G-code metadata in TinyGL renderer
- ✅ Camera preset to exact test angle (Az=85.5°, El=-2.5°, Zoom=10.0x)

**Files Modified:**
- `include/gcode_parser.h:163` - Added `layer_height_mm` field
- `include/gcode_geometry_builder.h:256-265, 309` - Added `set_layer_height()` method
- `src/gcode_geometry_builder.cpp:453-455, 498-502` - Layer height usage, corrected normals
- `src/gcode_tinygl_renderer.cpp:364-366, 540-559` - Layer height config, camera overlay
- `src/gcode_camera.cpp:28-38` - Exact test camera angle

### Recently Completed (Previous Sessions)

**Bed Mesh Grid Lines Implementation:**
- ✅ Wireframe grid overlay on mesh surface
- ✅ Dark gray (80,80,80) lines with 60% opacity
- ✅ LVGL 9.4 layer-based drawing (lv_canvas_init_layer/finish_layer)
- ✅ Coordinate system matches mesh quads exactly (Y-inversion, Z-centering)
- ✅ Horizontal and vertical lines connecting mesh vertices
- ✅ Bounds checking with -10px margin for partially visible lines
- ✅ Defensive checks for invalid canvas dimensions during flex layout

**Commits:**
- `dc2742e` - feat(bed_mesh): implement wireframe grid lines over mesh surface
- `e97b141` - refactor(bed_mesh): remove frontend fallback, use theme constants
- `e196b48` - fix(bed_mesh): dynamic canvas buffer and manual label updates
- `6ebf122` - fix(bed_mesh): reactive bindings now working with nullptr prev buffer

---

## 🚀 NEXT PRIORITIES

### 1. **FIX G-Code Tube Geometry (CRITICAL - BLOCKING)**

**Status:** Major geometry bugs discovered via debug coloring system

**Problem:** Rectangular tubes rendering twisted/kinked instead of straight

**Investigation Tasks:**
1. [ ] Add vertex position debug logging to `generate_ribbon_vertices()`
2. [ ] Verify start/end cap vertex alignment (8 vertices each)
3. [ ] Check triangle strip winding in face construction (lines 697-708)
4. [ ] Validate perpendicular vector calculation for horizontal lines
5. [ ] Test with multiple simple cases (horizontal, vertical, diagonal lines)

**Potential Fixes:**
- Ensure start_cap and end_cap vertices have identical ordering
- Verify cross-section calculation doesn't introduce rotation
- Check if vertex reuse (`prev_start_cap`) preserves order correctly
- Consider separating vertex positions from normal assignments

**Test Command:**
```bash
./build/bin/helix-ui-proto -p gcode-test -vv --test
# Camera should show: Az: 85.5° El: -2.5° Zoom: 10.0x
# Expected: Straight rectangular tube 10mm long
```

**Reference:**
- Screenshot: `/tmp/ui-screenshot-gcode-debug-colors.png` - Shows twisted tube
- Test file: `assets/single_line_test.gcode` - Single 10mm horizontal line
- Code: `src/gcode_geometry_builder.cpp:451-720` - Vertex generation

### 2. **Disable Debug Coloring After Fix**

Once geometry is corrected:
- [ ] Set `debug_face_colors(false)` in `gcode_tinygl_renderer.cpp:31`
- [ ] Or make it runtime-configurable via UI toggle
- [ ] Keep implementation for future debugging

### 3. **Complete Bed Mesh UI Features** (MEDIUM PRIORITY)

**Goal:** Match feature parity with GuppyScreen bed mesh visualization

**Completed:**
- ✅ Rotation sliders (Tilt/Spin) - visible and functional
- ✅ Info labels (dimensions, Z range) - visible with reactive bindings
- ✅ Grid lines - wireframe overlay properly aligned

**Remaining Tasks:**
1. [ ] Add axis labels (X/Y/Z indicators, bed dimensions)
2. [ ] Add mesh profile selector dropdown
3. [ ] Display additional statistics (variance/deviation)

### 4. **Integrate G-code Viewer with Print Select Panel**

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

### CRITICAL: G-Code Tube Geometry Bugs (BLOCKING)

**Discovered:** 2025-11-20 via per-face debug coloring system

**Symptoms:**
- Tubes render twisted/kinked instead of straight
- Black triangular gaps in geometry
- Wrong overall shape despite correct layer height and normals

**Evidence:**
- Screenshot: `/tmp/ui-screenshot-gcode-debug-colors.png`
- Test case: `assets/single_line_test.gcode` (10mm horizontal line)
- Camera angle: Az=85.5°, El=-2.5°, Zoom=10.0x

**Debug System Active:**
- Per-face coloring enabled (Top=Red, Bottom=Blue, Left=Green, Right=Yellow, StartCap=Magenta, EndCap=Cyan)
- Colors render correctly, confirming geometry generation is the issue
- Run with: `./build/bin/helix-ui-proto -p gcode-test -vv --test`

**Investigation Focus:**
- `src/gcode_geometry_builder.cpp:451-720` - `generate_ribbon_vertices()`
- Vertex ordering in TubeCap (8 vertices with duplicated positions, different normals)
- Triangle strip winding (lines 697-708)
- Cross-section calculation for horizontal lines

**Impact:** Blocks all G-code 3D visualization work

---

### Other Known Issues

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

**G-Code Geometry Issues:**
```bash
# Run with debug logging
./build/bin/helix-ui-proto -p gcode-test -vv --test

# Check for debug output
grep "DEBUG FACE COLORS" /tmp/gcode-debug.log
grep "Setting debug face colors" /tmp/gcode-debug.log

# Camera overlay should show:
# "Az: 85.5° El: -2.5° Zoom: 10.0x" (upper-left, only with -vv)
```

**Screenshot Current State:**
```bash
# Take screenshot (may open new instance)
./scripts/screenshot.sh helix-ui-proto gcode-debug gcode-test

# View screenshot
open /tmp/ui-screenshot-gcode-debug.png
```

**Add Vertex Debug Logging:**
```cpp
// In generate_ribbon_vertices() after vertex creation
spdlog::debug("Start cap vertices:");
for (int i = 0; i < 8; i++) {
    const auto& v = geometry.vertices[start_cap[i]];
    spdlog::debug("  [{}] pos=({},{},{})", i,
        quant.dequantize(v.position.x, quant.min_bounds.x),
        quant.dequantize(v.position.y, quant.min_bounds.y),
        quant.dequantize(v.position.z, quant.min_bounds.z));
}
```

**Next Session:** PRIORITY #1 is fixing the tube geometry bugs. Use the per-face debug coloring to diagnose vertex ordering and strip winding issues. Once geometry is correct, can proceed with bed mesh features and G-code viewer integration.
