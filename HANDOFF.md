# Session Handoff Document

**Last Updated:** 2025-11-14
**Current Focus:** Bed Mesh Visualization + G-code 3D Visualization

---

## ✅ CURRENT STATE

### Just Completed (This Session)

**Bed Mesh UI Fixes & Reactive Bindings:**
- ✅ Fixed reactive label bindings (nullptr prev buffer - wizard pattern)
- ✅ Info labels now show real mesh data: "7x7 points", "Z: -0.300 to 0.288 mm"
- ✅ Fixed mesh positioning (dynamic canvas buffer reallocation on SIZE_CHANGED)
- ✅ Mesh now uses full available canvas space (624x223px responsive)
- ✅ Removed frontend test data fallback - all data from Moonraker (mock or real)
- ✅ XML refactored with theme constants (#padding_*, #card_bg)
- ✅ Panel properly centered, no clipping

**Commits:**
- `e97b141` - refactor(bed_mesh): remove frontend fallback, use theme constants
- `e196b48` - fix(bed_mesh): dynamic canvas buffer and manual label updates
- `6ebf122` - fix(bed_mesh): reactive bindings now working with nullptr prev buffer

**Key Fix:** `lv_subject_init_string(&subj, buf, nullptr, ...)` - using nullptr for prev_buf parameter (not separate buffer) matches wizard pattern and allows proper observer notifications.

### Recently Completed (Previous Sessions)

1. **G-Code 3D Visualization System - Phase 1** ✅ COMPLETE
   - Streaming G-code parser with layer indexing
   - 3D-to-2D renderer with LVGL canvas drawing
   - Interactive 3D camera system (spherical coordinates, touch rotation)
   - Custom LVGL widget (`ui_gcode_viewer`) with XML registration
   - Test panel accessible via `-p gcode-test`
   - Complete documentation in `docs/GCODE_VISUALIZATION.md`
   - 14 comprehensive unit tests

2. **Bed Mesh Phase 1: Settings Panel Infrastructure** - ✅ COMPLETE
   - Created settings_panel.xml with 6-card launcher grid
   - Only "Bed Mesh" card is active, opens visualization panel

3. **Bed Mesh Phase 2: Core 3D Rendering Engine** - ✅ COMPLETE
   - Comprehensive bed_mesh_renderer.h/cpp (768 lines, C API)
   - Perspective projection, triangle rasterization, depth sorting
   - Heat-map color mapping (purple→blue→cyan→yellow→red)
   - Analysis documented in docs/GUPPYSCREEN_BEDMESH_ANALYSIS.md

4. **Bed Mesh Phase 3: Basic Visualization** - ✅ COMPLETE
   - bed_mesh_panel.xml created (overlay panel structure)
   - Canvas (600×400 RGB888) renders gradient mesh correctly
   - Test mesh: 7×7 dome shape (mock backend)
   - Back button works, resource cleanup on panel deletion

5. **Bed Mesh Phase 4: Moonraker Integration** - ✅ COMPLETE
   - Added BedMeshProfile struct to moonraker_client.h
   - Implemented parse_bed_mesh() for WebSocket updates
   - Added reactive subjects: bed_mesh_dimensions, bed_mesh_z_range
   - Mock backend generates 7×7 synthetic dome mesh
   - Real-time updates via notify_status_update callback

### What Works Now

**G-code Visualization:**
- ✅ Parse G-code files with layer-indexed data structure
- ✅ 3D orthographic rendering on LVGL canvas
- ✅ Interactive camera rotation (touch drag gestures)
- ✅ Preset views (Isometric, Top, Front, Side)
- ✅ Theme-integrated color coding (extrusion vs travel moves)
- ✅ Command-line access: `./build/bin/helix-ui-proto -p gcode-test`

**Bed Mesh Visualization:**
- ✅ Settings panel → Bed Mesh card → Visualization panel
- ✅ Gradient mesh rendering (heat-map colors)
- ✅ Moonraker integration (fetches real bed mesh data, mock provides 7x7 dome)
- ✅ Reactive subjects update info labels ("7x7 points", "Z: -0.300 to 0.288 mm")
- ✅ Widget encapsulation (proper lifecycle management)
- ✅ Bounds checking (no coordinate errors)
- ✅ Responsive canvas buffer (dynamically resizes with flex layout)
- ✅ Rotation sliders functional (Tilt: -45°, Spin: 45°)
- ✅ Theme constants used throughout XML

### 🚨 What's MISSING (Critical UI Features)

**Bed Mesh - Gradient mesh works, but UI needs enhancement:**

❌ **Grid Lines** - Not implemented
   - Reference grid over mesh surface (XY plane)
   - Should show mesh cell boundaries

❌ **Axis Labels** - Not implemented
   - X/Y/Z axis indicators
   - Dimension labels (bed size, probe spacing)

❌ **Info Labels** - Partially implemented
   - XML has `mesh_dimensions_label` and `mesh_z_range_label` with reactive bindings
   - **NEED TO VERIFY:** Are they actually visible? Correct positioning?
   - May need styling, positioning, or visibility fixes

❌ **Rotation Sliders** - Partially implemented
   - XML has `rotation_x_slider` and `rotation_z_slider`
   - XML has `rotation_x_label` and `rotation_y_label`
   - Wired to C++ callbacks (rotation_x_slider_cb, rotation_z_slider_cb)
   - **NEED TO VERIFY:** Are they visible and functional?
   - May need default values, styling, or layout fixes

❌ **Mesh Profile Selector** - Not implemented
   - Dropdown to switch between mesh profiles (default, adaptive, etc.)
   - Should show available profiles from Moonraker

❌ **Mesh Statistics** - Not implemented
   - Min/Max Z values (implemented in backend, not displayed)
   - Mesh variance/deviation
   - Probe count/density

---

## 🚀 NEXT PRIORITIES

### 1. **Complete Bed Mesh UI Features** (HIGH PRIORITY)

**Goal:** Match feature parity with GuppyScreen bed mesh visualization

**Tasks:**

1. **Verify & Fix Existing UI Elements**
   - [ ] Check if rotation sliders are visible and functional
   - [ ] Check if info labels (dimensions, Z range) are visible
   - [ ] Fix positioning/styling if elements are off-screen or hidden
   - [ ] Test rotation slider interaction (does mesh rotate?)

2. **Implement Grid Lines** (in renderer)
   - [ ] Add grid rendering to bed_mesh_renderer.cpp
   - [ ] Draw XY plane grid at Z=0 or mesh_min_z
   - [ ] Grid spacing based on mesh dimensions
   - [ ] Grid lines in contrasting color (white/light gray)

3. **Implement Axis Labels** (in renderer or XML)
   - [ ] Add X/Y/Z axis indicators
   - [ ] Label bed dimensions (min/max X, min/max Y)
   - [ ] Show probe spacing if available

4. **Add Mesh Profile Selector** (XML + C++)
   - [ ] Add dropdown to bed_mesh_panel.xml
   - [ ] Populate from client->get_bed_mesh_profiles()
   - [ ] Wire onChange to load selected profile
   - [ ] Show active profile name

5. **Add Mesh Statistics Display** (XML + reactive bindings)
   - [ ] Min/Max Z (already computed, need display)
   - [ ] Mesh variance/deviation
   - [ ] Probe count (already available)
   - [ ] Use reactive subjects for auto-update

**Reference Implementation:**
- GuppyScreen: `panels/bed_mesh_panel.py` (grid, axes, labels)
- Existing: `bed_mesh_renderer.cpp` (add grid rendering here)
- Existing: `bed_mesh_panel.xml` (add missing UI elements here)

### 2. **Integrate G-code Viewer with Print Select Panel**

**Goal:** Add "Preview" button to print file browser

**Tasks:**
- [ ] Add preview button/icon to file list items in print_select_panel.xml
- [ ] Fetch G-code file via Moonraker HTTP API (not WebSocket)
- [ ] Open G-code viewer in overlay panel
- [ ] Show filename and basic stats (layers, print time if available)
- [ ] Allow returning to file list

**Reference:**
- `docs/GCODE_VISUALIZATION.md` - Integration guide
- `ui_panel_gcode_test.cpp` - Example usage of viewer widget

### 3. **Grid Rendering Implementation Details**

**Where:** Add to `bed_mesh_renderer.cpp` after mesh rendering

**Approach:**
```cpp
// After rendering quads, draw grid lines
if (show_grid) {
    draw_grid_lines(canvas, mesh, canvas_width, canvas_height, view_state);
}
```

**Grid should:**
- Draw at Z=0 plane or slightly below mesh
- Use mesh cell boundaries (rows-1 × cols-1 cells)
- Project to screen space using same projection as mesh
- Draw with lv_canvas_draw_line() or pixel-by-pixel

### 4. **UI Elements to Add in XML**

**bed_mesh_panel.xml needs:**
```xml
<!-- Mesh profile selector (if multiple profiles available) -->
<lv_dropdown name="mesh_profile_selector" bind_text="bed_mesh_profile_name"/>

<!-- Mesh statistics card -->
<lv_obj> <!-- stats container -->
  <lv_label bind_text="bed_mesh_min_z"/>
  <lv_label bind_text="bed_mesh_max_z"/>
  <lv_label bind_text="bed_mesh_variance"/>
</lv_obj>
```

**C++ subjects to add:**
- `bed_mesh_min_z` (already computed, need subject)
- `bed_mesh_max_z` (already computed, need subject)
- `bed_mesh_variance`

---

## 📋 Critical Patterns Reference

### Pattern #0: LV_SIZE_CONTENT Bug

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

### Pattern #1: Bed Mesh Widget API

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

### Pattern #2: Reactive Subjects for Mesh Data

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

### Pattern #3: Moonraker Bed Mesh Access

```cpp
#include "app_globals.h"
#include "moonraker_client.h"

MoonrakerClient* client = get_moonraker_client();
if (!client || !client->has_bed_mesh()) {
    // Fall back to test mesh
    return;
}

const auto& mesh = client->get_active_bed_mesh();
// mesh.probed_matrix - 2D vector<vector<float>>
// mesh.x_count, mesh.y_count - dimensions
// mesh.mesh_min[2], mesh.mesh_max[2] - bounds
// mesh.name - profile name

const auto& profiles = client->get_bed_mesh_profiles();
// vector<string> of available profile names
```

### Pattern #4: Thread Management - NEVER Block UI Thread

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

### Pattern #5: G-code Viewer Widget API

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

1. **Missing Bed Mesh UI Features** (see "What's MISSING" section above)
   - Grid lines not implemented
   - Axis labels not implemented
   - Info labels may not be visible
   - Rotation sliders may not be working
   - Only gradient mesh currently visible

2. **No Bed Mesh Profile Switching**
   - Can fetch multiple profiles from Moonraker
   - No UI to switch between profiles

3. **Limited Bed Mesh Metadata Display**
   - Min/Max Z computed but not shown prominently
   - No variance/deviation statistics

4. **G-code Viewer Not Integrated**
   - Standalone test panel works (`-p gcode-test`)
   - Not yet integrated with print select panel
   - No "Preview" button in file browser

**Next Session:** Focus on completing missing bed mesh UI features, starting with verifying/fixing rotation sliders and info labels, then implementing grid lines. Consider integrating G-code viewer with print select panel.
