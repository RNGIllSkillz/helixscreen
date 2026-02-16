# Namespace Refactoring Design

**Date:** 2026-02-16
**Branch:** `refactor/namespaces`
**Approach:** Big bang — move all declarations, update all call sites, no backward-compat aliases

## Scope

### HIGH Priority

| Category | Items | Est. Files |
|----------|-------|-----------|
| Core singletons → `helix::` | `PrinterState`, `MoonrakerClient`, `Config`, `SettingsManager` | ~400 |
| C-style enums → `enum class` | 11 typedef enums in UI headers | ~25 |
| UI free functions → `helix::ui::` | `ui_modal_*`, `ui_queue_update`, `ui_status_bar_*`, `lv_obj_safe_delete`, etc. (drop `ui_` prefix) | ~120 |
| Callback typedefs → `helix::` | ~30+ global `using XCallback = std::function<...>` | ~50 |

### MEDIUM Priority

| Category | Items | Est. Files |
|----------|-------|-----------|
| Manager classes → `helix::` | `PanelFactory`, `WiFiManager`, `SoundManager`, `TipsManager`, `MdnsDiscovery` | ~40 |
| JSON alias consolidation | 10 headers with `using json = nlohmann::json` → single common header | ~10 headers |

### NOT in scope

- LOW priority conversion functions (`ams_type_to_string`, etc.)
- Sub-namespace splitting (`helix::network`, `helix::audio`) — flat `helix::` for now

## Naming Conventions

### Singletons & Classes

No name changes, just namespace wrapping:

```cpp
// Before
class PrinterState { ... };
// After
namespace helix { class PrinterState { ... }; }
```

### UI Free Functions

Drop `ui_` prefix since `helix::ui::` makes it redundant:

| Current | New |
|---------|-----|
| `ui_queue_update(cb)` | `helix::ui::queue_update(cb)` |
| `ui_modal_hide(dialog)` | `helix::ui::modal_hide(dialog)` |
| `ui_modal_is_visible()` | `helix::ui::modal_is_visible()` |
| `ui_status_bar_init()` | `helix::ui::status_bar_init()` |
| `ui_event_safe_call(...)` | `helix::ui::event_safe_call(...)` |
| `lv_obj_safe_delete(obj)` | `helix::ui::safe_delete(obj)` |
| `ui_toggle_list_empty_state(...)` | `helix::ui::toggle_list_empty_state(...)` |

### C-style Enums → enum class

| Current | New |
|---------|-----|
| `typedef enum { NETWORK_WIFI, ... } network_type_t` | `enum class NetworkType { Wifi, Ethernet, Disconnected }` in `helix::` |
| `typedef enum { JOG_DIRECTION_X_POSITIVE, ... } jog_direction_t` | `enum class JogDirection { XPositive, ... }` in `helix::` |
| `typedef enum { HEATER_TYPE_EXTRUDER, ... } ui_heater_type_t` | `enum class HeaterType { Extruder, ... }` in `helix::` |
| `typedef enum { STEP_STATE_PENDING, ... } ui_step_state_t` | `enum class StepState { Pending, ... }` in `helix::ui` |
| `typedef enum { BED_MESH_RENDER_MODE_3D, ... } bed_mesh_render_mode_t` | `enum class BedMeshRenderMode { Mode3D, ... }` in `helix::` |
| G-code viewer enums | `enum class GcodeViewerViewMode`, `GcodeViewerShowMode`, etc. in `helix::` |

### Callback Typedefs

Move into `helix::` or into the relevant class if tightly coupled:

```cpp
// Before (global)
using MacroListCallback = std::function<void(const std::vector<std::string>&)>;
// After
namespace helix { using MacroListCallback = std::function<void(const std::vector<std::string>&)>; }
```

### JSON Alias

Create `include/json_fwd.h` with the single definition:

```cpp
#pragma once
#include "hv/json.hpp"
using json = nlohmann::json;
```

All other headers replace their local `using json =` with `#include "json_fwd.h"`.

## Implementation Strategy

### .cpp File Convention

To minimize diff churn in implementation files, add `using namespace helix;` (and `using namespace helix::ui;` where needed) at the top of .cpp files rather than qualifying every call. Namespace discipline is enforced at the header level.

### Execution Order

1. **JSON consolidation** — create `json_fwd.h`, update 10 headers (low risk, unblocks nothing)
2. **Enum modernization** — convert typedef enums to enum class (small blast radius, 2-6 files each)
3. **Manager classes** — wrap in `helix::` (5-12 files each)
4. **Callback typedefs** — move into `helix::` (~50 files)
5. **UI free functions** — move to `helix::ui::`, rename (~120 files)
6. **Core singletons** — wrap in `helix::` (~400 files, biggest change last)

Each step should compile and pass tests before moving to the next.
