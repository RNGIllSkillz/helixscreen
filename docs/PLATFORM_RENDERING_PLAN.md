# Platform-Specific Rendering Architecture

## Overview

Implement platform-appropriate rendering for HelixScreen based on hardware capabilities:
- **Raspberry Pi**: Full LVGL OpenGL ES rendering (GPU-accelerated), 3D visualizations
- **Adventurer 5M**: Disable TinyGL, NEON-optimized 2D rendering, **2D heatmap + layer view**
- **Desktop (SDL)**: Unchanged (TinyGL continues to work)

## Background

AD5M has Allwinner R528 SoC (Dual Cortex-A7, **NO GPU**, 37MB free RAM). TinyGL produces 3-4 FPS - unusable.

## Architecture Summary

```
┌─────────────────────────────────────────────────────┐
│                   LVGL 9.x UI                       │
└─────────────────────┬───────────────────────────────┘
                      │
        ┌─────────────┼─────────────┐
        │             │             │
        ▼             ▼             ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│   OpenGL ES  │ │  2D Fallback │ │   TinyGL     │
│  (Pi + GPU)  │ │   (AD5M)     │ │   (Desktop)  │
├──────────────┤ ├──────────────┤ ├──────────────┤
│ • 3D bed mesh│ │ • 2D heatmap │ │ • 3D preview │
│ • 3D g-code  │ │ • 2D layers  │ │ • Dev/test   │
│ • 30+ FPS    │ │ • 15+ FPS    │ │              │
└──────────────┘ └──────────────┘ └──────────────┘
```

## Git Setup & Plan Persistence

**IMPORTANT**: This plan lives in `docs/PLATFORM_RENDERING_PLAN.md` in the worktree for cross-session reference.

```bash
# Step 1: Create feature branch and worktree
git checkout -b feature/platform-render-architecture
git worktree add ../helixscreen-render feature/platform-render-architecture

# Step 2: Copy this plan to docs/ for persistence
cd ../helixscreen-render
cp ~/.claude/plans/lucky-marinating-thimble.md docs/PLATFORM_RENDERING_PLAN.md
git add docs/PLATFORM_RENDERING_PLAN.md
git commit -m "docs: add platform rendering architecture plan"

# Step 3: Work from the worktree
# All implementation happens here, updating the plan after each phase
```

### Plan Update Workflow

After completing each phase:
1. Update `docs/PLATFORM_RENDERING_PLAN.md` with status
2. Mark completed items with ✅
3. Add any learnings or adjustments
4. Commit the updated plan

---

## Phase 1: AD5M - Disable TinyGL (Simple)

### 1.1 Build System Change

**File: `mk/cross.mk` (line 69)**
```diff
- ENABLE_TINYGL_3D := yes
+ ENABLE_TINYGL_3D := no
```

### 1.2 Enable LVGL NEON Blending

**File: `lv_conf.h` (line 184)**
```diff
- #define  LV_USE_DRAW_SW_ASM     LV_DRAW_SW_ASM_NONE
+ /* Enable NEON SIMD for ARM platforms with NEON support */
+ #if defined(__ARM_NEON) || defined(__ARM_NEON__)
+     #define  LV_USE_DRAW_SW_ASM     LV_DRAW_SW_ASM_NEON
+ #else
+     #define  LV_USE_DRAW_SW_ASM     LV_DRAW_SW_ASM_NONE
+ #endif
```

### 1.3 Verification
```bash
make ad5m-docker
# Check no TinyGL symbols: nm build/ad5m/bin/helix-screen | grep -i tinygl
# Deploy and verify 2D G-code preview works
```

---

## Phase 2: Raspberry Pi - OpenGL ES Integration (Complex)

### 2.1 Docker Toolchain Update

**File: `docker/Dockerfile.pi`** - Add OpenGL ES libraries:
```dockerfile
RUN dpkg --add-architecture arm64 && apt-get update && apt-get install -y --no-install-recommends \
    # ... existing packages ...
    libgbm-dev:arm64 \
    libgles2-mesa-dev:arm64 \
    libegl1-mesa-dev:arm64 \
    && rm -rf /var/lib/apt/lists/*
```

### 2.2 Build System Changes

**File: `mk/cross.mk`** - Add OpenGL ES flag for Pi (after line 34):
```makefile
ifeq ($(PLATFORM_TARGET),pi)
    # ... existing settings ...
    ENABLE_OPENGLES := yes
endif
```

**File: `mk/cross.mk`** - Add defines (around line 166, after display backend section):
```makefile
# OpenGL ES support for GPU-accelerated rendering
ifeq ($(ENABLE_OPENGLES),yes)
    CFLAGS += -DHELIX_ENABLE_OPENGLES
    CXXFLAGS += -DHELIX_ENABLE_OPENGLES
    SUBMODULE_CFLAGS += -DHELIX_ENABLE_OPENGLES
    SUBMODULE_CXXFLAGS += -DHELIX_ENABLE_OPENGLES
endif
```

**File: `Makefile`** - Add linker flags for Pi (in cross-compile section):
```makefile
ifeq ($(PLATFORM_TARGET),pi)
    ifeq ($(ENABLE_OPENGLES),yes)
        LDFLAGS += -lGLESv2 -lEGL -lgbm
    endif
endif
```

### 2.3 LVGL Configuration

**File: `lv_conf.h`** - Enable DRM+EGL and OpenGL ES draw backend:

Near the DRM section (around line 1078):
```c
#ifdef HELIX_DISPLAY_DRM
    #define LV_USE_LINUX_DRM        1

    /* Enable EGL/OpenGL ES on Pi for GPU acceleration */
    #ifdef HELIX_ENABLE_OPENGLES
        #define LV_LINUX_DRM_USE_EGL         1
        #define LV_USE_LINUX_DRM_GBM_BUFFERS 1
    #else
        #define LV_LINUX_DRM_USE_EGL         0
    #endif
#else
    #define LV_USE_LINUX_DRM        0
#endif
```

Near the draw backends section (after LV_USE_DRAW_SDL):
```c
/* Use OpenGL ES draw backend for Pi GPU acceleration */
#ifdef HELIX_ENABLE_OPENGLES
    #define LV_USE_DRAW_OPENGLES 1
    #if LV_USE_DRAW_OPENGLES
        #define LV_DRAW_OPENGLES_TEXTURE_CACHE_COUNT 64
    #endif
#else
    #define LV_USE_DRAW_OPENGLES 0
#endif
```

### 2.4 Verification
```bash
# Rebuild Docker toolchain with new libraries
make docker-toolchain-pi

# Cross-compile with OpenGL ES
make pi-docker

# Deploy and test
make deploy-pi-fg
# Look for: "[info] Egl version 1.4" and "[OPENGLES]" in logs
```

---

## Phase 3: Testing & Validation

### Build Matrix
| Target | Command | Expected Result |
|--------|---------|-----------------|
| Native/SDL | `make -j` | TinyGL works, unchanged |
| AD5M | `make ad5m-docker` | No TinyGL, NEON blending enabled |
| Pi | `make pi-docker` | OpenGL ES acceleration |

### Visual Verification
- [ ] Native: G-code 3D preview renders (TinyGL)
- [ ] AD5M: G-code shows 2D layer preview or thumbnail
- [ ] Pi: G-code 3D preview renders with GPU acceleration
- [ ] All platforms: Touch input works
- [ ] All platforms: Dark/light mode switching works

### Performance Targets
| Platform | Before | After |
|----------|--------|-------|
| AD5M G-code | 3-4 FPS (TinyGL) | 15-20 FPS (2D + NEON) |
| Pi UI | 20-30 FPS (SW) | 30-60 FPS (OpenGL ES) |

---

## Phase 4: 2D Bed Mesh Heatmap (Runtime Detection)

**Goal**: Single renderer that auto-detects platform and uses 3D (Pi) or 2D heatmap (AD5M).

### 4.1 Add Platform Detection

**File: `include/platform_capabilities.h`** (new):
```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace helix {

enum class RenderingCapability {
    HARDWARE_OPENGL,  // Pi with GPU
    SOFTWARE_NEON,    // AD5M with NEON
    SOFTWARE_BASIC    // Fallback
};

RenderingCapability detect_rendering_capability();
bool has_gpu_support();

} // namespace helix
```

### 4.2 Modify Bed Mesh Renderer

**File: `src/bed_mesh_renderer.cpp`** - Add rendering mode:
```cpp
enum class BedMeshRenderMode { MODE_3D, MODE_2D_HEATMAP };

// At render time:
if (helix::has_gpu_support()) {
    render_3d_perspective(...);  // Existing 3D code
} else {
    render_2d_heatmap(...);      // New simplified path
}
```

### 4.3 Implement 2D Heatmap Rendering

**New function in `bed_mesh_renderer.cpp`**:
```cpp
void render_2d_heatmap(lv_layer_t* layer, const BedMeshData& mesh) {
    // Simple grid of colored rectangles
    // - Uniform cell size based on mesh dimensions
    // - Color gradient: blue (low) → green → yellow → red (high)
    // - No perspective, no rotation
    // - Touch shows Z value tooltip
}
```

**Features**:
- Grid of `mesh_width × mesh_height` rectangles
- Color mapped from Z deviation: purple (-) → green (0) → red (+)
- Cell labels showing Z value on touch/hover
- 60+ FPS on AD5M (no 3D math)

### 4.4 Files to Modify

| File | Change |
|------|--------|
| `include/platform_capabilities.h` | New - runtime detection |
| `src/platform_capabilities.cpp` | New - detection implementation |
| `src/bed_mesh_renderer.cpp` | Add `render_2d_heatmap()` function |
| `src/ui_bed_mesh.cpp` | Remove rotation handlers when 2D mode |

**Estimated effort**: 1-2 days

---

## Phase 5: 2D G-code Layer View (Basic)

**Goal**: Top-down orthographic view of single layer with extrusion/travel coloring.

### 5.1 Create Layer Preview Renderer

**File: `include/gcode_layer_renderer.h`** (new):
```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class GCodeLayerRenderer {
public:
    void set_gcode(const ParsedGCodeFile* gcode);
    void set_current_layer(int layer);
    void render(lv_layer_t* layer, const lv_area_t* area);

    // Options
    void set_show_travels(bool show);
    void set_extrusion_color(lv_color_t color);
    void set_travel_color(lv_color_t color);

private:
    const ParsedGCodeFile* gcode_ = nullptr;
    int current_layer_ = 0;
    bool show_travels_ = false;

    // Viewport/zoom
    float scale_ = 1.0f;
    float offset_x_ = 0.0f;
    float offset_y_ = 0.0f;

    void compute_layer_bounds();
    void render_segment_2d(lv_layer_t* layer, const ToolpathSegment& seg);
};
```

### 5.2 Rendering Approach

**Top-down orthographic projection**:
```cpp
void GCodeLayerRenderer::render_segment_2d(lv_layer_t* layer, const ToolpathSegment& seg) {
    // Direct X/Y mapping (ignore Z for 2D view)
    int x1 = (seg.start.x - offset_x_) * scale_;
    int y1 = (seg.start.y - offset_y_) * scale_;
    int x2 = (seg.end.x - offset_x_) * scale_;
    int y2 = (seg.end.y - offset_y_) * scale_;

    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = seg.is_extrusion ? color_extrusion_ : color_travel_;
    dsc.width = seg.is_extrusion ? 2 : 1;
    dsc.opa = seg.is_extrusion ? LV_OPA_COVER : LV_OPA_50;

    lv_draw_line(layer, &dsc, x1, y1, x2, y2);
}
```

### 5.3 Integrate with UI

**File: `src/ui_gcode_viewer.cpp`** - Add mode switching:
```cpp
// Runtime selection based on platform
if (helix::has_gpu_support()) {
    // Pi: Use existing 3D renderer (TinyGL or OpenGL ES)
    renderer_3d_->render(layer, gcode, camera);
} else {
    // AD5M: Use new 2D layer renderer
    layer_renderer_->set_current_layer(current_layer_);
    layer_renderer_->render(layer, area);
}
```

### 5.4 UI Controls Needed

**Layer slider** (already exists in gcode_test_panel.xml):
- Min: 0, Max: total_layers - 1
- Updates `layer_renderer_->set_current_layer()`

**Layer info display**:
- Current layer number / total layers
- Layer height (Z)
- Segment count

### 5.5 Files to Create/Modify

| File | Change |
|------|--------|
| `include/gcode_layer_renderer.h` | New - 2D layer renderer interface |
| `src/gcode_layer_renderer.cpp` | New - implementation |
| `src/ui_gcode_viewer.cpp` | Add runtime mode selection |
| `ui_xml/gcode_viewer_panel.xml` | Add layer slider (if not present) |

**Estimated effort**: 2-3 days

---

## Critical Files Summary

### Phase 1-3 (Build/Config)
| File | Change |
|------|--------|
| `mk/cross.mk:69` | AD5M: `ENABLE_TINYGL_3D := no` |
| `mk/cross.mk:34+` | Pi: Add `ENABLE_OPENGLES := yes` |
| `mk/cross.mk:166+` | Add `HELIX_ENABLE_OPENGLES` defines |
| `lv_conf.h:184` | Conditional NEON ASM |
| `lv_conf.h:1078+` | DRM+EGL configuration |
| `lv_conf.h:238+` | OpenGL ES draw backend |
| `docker/Dockerfile.pi` | Add EGL/GLES/GBM packages |
| `Makefile` | Add -lGLESv2 -lEGL -lgbm for Pi |

### Phase 4 (2D Bed Mesh)
| File | Change |
|------|--------|
| `include/platform_capabilities.h` | New - runtime GPU detection |
| `src/platform_capabilities.cpp` | New - detection implementation |
| `src/bed_mesh_renderer.cpp` | Add `render_2d_heatmap()` |
| `src/ui_bed_mesh.cpp` | Conditional rotation handlers |

### Phase 5 (2D G-code Layer)
| File | Change |
|------|--------|
| `include/gcode_layer_renderer.h` | New - 2D layer renderer |
| `src/gcode_layer_renderer.cpp` | New - implementation |
| `src/ui_gcode_viewer.cpp` | Runtime mode selection |

---

## Rollback Plan

If issues discovered:
1. **Pre-merge**: Reset feature branch, fix issues
2. **Post-merge**: `git revert <commit-sha>`
3. **Platform-specific**: Create hotfix branch for targeted fix

## Implementation Order

| Phase | Description | Effort | Risk |
|-------|-------------|--------|------|
| 1 | AD5M: Disable TinyGL + NEON | 1-2 hours | Low |
| 2 | Pi: OpenGL ES integration | 2-4 hours | Medium |
| 3 | Testing & validation | 1-2 hours | Low |
| 4 | 2D Bed Mesh Heatmap | 1-2 days | Low |
| 5 | 2D G-code Layer View | 2-3 days | Medium |

**Total estimated effort**: ~1 week

**Recommended approach**: Complete Phases 1-3 first, verify builds work, then proceed to Phases 4-5.

---

## Status Tracking

Update this section after each phase:

| Phase | Status | Started | Completed | Notes |
|-------|--------|---------|-----------|-------|
| 1 | ⬜ Not Started | - | - | |
| 2 | ⬜ Not Started | - | - | |
| 3 | ⬜ Not Started | - | - | |
| 4 | ⬜ Not Started | - | - | |
| 5 | ⬜ Not Started | - | - | |

**Status Legend**: ⬜ Not Started | 🔄 In Progress | ✅ Complete | ⚠️ Blocked

### Session Log

Record progress across sessions:

```
<!-- Template:
### Session YYYY-MM-DD
- Phase X: [status]
- Changes made: [list]
- Issues encountered: [list]
- Next steps: [list]
-->
```
