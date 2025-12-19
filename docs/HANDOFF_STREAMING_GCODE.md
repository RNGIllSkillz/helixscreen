# Handoff Prompt: Streaming G-Code Phase 7 (Background Ghost Caching)

## Context

The streaming G-code architecture for HelixScreen enables viewing 10MB+ G-code files on memory-constrained devices (AD5M with 47MB RAM).

## Completed Phases

| Phase | Commit | Description |
|-------|--------|-------------|
| 1 | bed0255 | `GCodeStreamingConfig` (3-level priority), `GCodeLayerIndex` (~24 bytes/layer) |
| 2-3 | 745ffe9 | `GCodeLayerCache` (LRU, adaptive memory), `GCodeDataSource` (File/Moonraker/Memory), configurable `layers_per_frame` |
| 4 | 745ffe9 | `GCodeStreamingController` - orchestrates all components |
| 5 | 745ffe9 | `GCodeLayerRenderer` integration - works with controller |
| 6 | pending | UI Integration - wired streaming into `ui_gcode_viewer.cpp` |

## Phase 6 Summary (Just Completed)

### What Was Done

1. **Streaming mode detection** in `ui_gcode_viewer_load_file_async()`:
   - Gets file size via `std::filesystem::file_size()`
   - Calls `helix::should_use_gcode_streaming(file_size)` to decide mode
   - Creates `GCodeStreamingController` for large files

2. **State management**:
   - Added `streaming_controller_` to `GCodeViewerState`
   - Helper methods: `is_streaming_mode()`, `has_gcode_data()`
   - Proper cleanup ordering to prevent dangling pointers

3. **Thread safety fixes**:
   - `lv_obj_is_valid(obj)` check before accessing widget in async callback
   - Mutex protection for callback in streaming controller
   - Stale callback detection for rapid file reloads

4. **X/Y bounding box tracking** in `GCodeLayerIndex`:
   - Index now tracks `min_x`, `max_x`, `min_y`, `max_y` during scan
   - Renderer uses actual bounds for auto_fit (matches non-streaming mode)

5. **Unit tests**: 124 gcode tests, 1731 assertions passing

### Current Limitations

- **Ghost mode disabled in streaming mode** - requires all layers loaded upfront
- **Prefetching** loads ±3 layers around current view

## Phase 7: Background Ghost Caching (NEXT)

### Goal

Enable ghost mode in streaming mode by progressively caching all layers in the background.

**User experience:**
1. File loads → streaming mode starts immediately (no ghost)
2. Background task progressively caches layers
3. Once all layers cached → ghost mode "magically" enables
4. User sees "Building preview..." indicator during caching

### Proposed Implementation

```cpp
// In GCodeStreamingController:
class BackgroundGhostBuilder {
public:
    void start(GCodeStreamingController* controller);
    void cancel();

    float get_progress() const;       // 0.0 to 1.0
    bool is_complete() const;
    bool is_running() const;

private:
    std::thread worker_;
    std::atomic<float> progress_{0.0f};
    std::atomic<bool> complete_{false};
    std::atomic<bool> cancelled_{false};
};

// In GCodeLayerRenderer:
void set_ghost_cache_progress(float progress);  // For UI indicator
void enable_ghost_when_ready();  // Auto-enable once cache is complete
```

### Key Changes Needed

1. **Background caching task**:
   - Iterate through all layers in low-priority background thread
   - Load each layer into cache (triggers parsing)
   - Update progress atomic for UI display

2. **Renderer integration**:
   - Track ghost cache readiness
   - Auto-enable ghost rendering once all layers are cached
   - Optional: partial ghost showing cached layers only

3. **UI indicator**:
   - Small progress bar or text: "Building preview: 45%"
   - Dismiss once complete

4. **Memory management**:
   - Background caching respects cache budget
   - May need larger budget temporarily during caching
   - Consider eviction policy during caching (don't evict what we just loaded)

### Constraints

- **Memory budget**: Must not exceed available RAM during caching
- **Priority**: Caching should be lower priority than user-requested layers
- **Cancellation**: Must be able to cancel on file change or app close

### Test Commands

```bash
# Run all gcode tests
make -j && ./build/bin/run_tests "[gcode]"

# Test streaming mode
HELIX_GCODE_STREAMING=on ./build/bin/helix-screen --test -p print-status --sim-speed 250 -vv
```

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    GCodeStreamingController                      │
│  (Facade orchestrating all streaming components)                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────────┐    ┌──────────────────┐                   │
│  │ GCodeLayerIndex  │    │ GCodeDataSource  │                   │
│  │ (~24 bytes/layer)│    │ (File/Moonraker) │                   │
│  │ + X/Y bounds     │    └────────┬─────────┘                   │
│  └────────┬─────────┘             │                              │
│           │    ┌──────────────────┘                              │
│           │    │                                                 │
│           ▼    ▼                                                 │
│  ┌──────────────────┐    ┌──────────────────┐                   │
│  │ GCodeLayerCache  │◄───│   GCodeParser    │                   │
│  │ (LRU, adaptive)  │    │ (bytes→segments) │                   │
│  └──────────────────┘    └──────────────────┘                   │
│           ▲                                                      │
│           │ (Phase 7)                                            │
│  ┌────────┴─────────┐                                            │
│  │BackgroundGhost   │                                            │
│  │   Builder        │                                            │
│  └──────────────────┘                                            │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                   ┌──────────────────┐
                   │GCodeLayerRenderer│
                   │  (2D rendering)  │
                   │  + ghost mode    │
                   └──────────────────┘
```

## Key Files

| File | Purpose |
|------|---------|
| `include/gcode_streaming_controller.h` | Controller API |
| `src/gcode_streaming_controller.cpp` | Controller implementation |
| `include/gcode_layer_renderer.h` | Renderer with `set_streaming_controller()` |
| `src/gcode_layer_renderer.cpp` | Renderer streaming integration |
| `src/ui_gcode_viewer.cpp` | UI widget with streaming support |
| `tests/unit/test_gcode_streaming_controller.cpp` | Controller tests |
| `tests/unit/test_gcode_streaming_ui_integration.cpp` | UI integration tests |

## Current Test Status

```
✓ 124 gcode test cases (123 passed, 1 skipped)
✓ 1731 assertions passed
✓ Streaming + UI integration working
```
