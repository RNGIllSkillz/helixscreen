# Session Handoff Document

**Last Updated:** 2025-10-31
**Current Focus:** LVGL theme breakpoint patch + comprehensive testing framework

---

## ✅ Recently Completed (Session 2025-10-31)

**LVGL Theme Breakpoint Customization (IN PROGRESS)**
- ✅ **LVGL theme patch created** - `patches/lvgl_theme_breakpoints.patch` (6 lines changed)
  - Breakpoints: 320→480 (SMALL), 720→800 (MEDIUM), hardware-aligned
  - Padding: PAD_DEF (12/16/20), PAD_SMALL (8/10/12), PAD_TINY (2/4/6)
- ✅ **Build system modularization** - Created `mk/patches.mk` for upstream patch management
  - Auto-applies both SDL window + theme breakpoints patches
  - Separated from fonts.mk (cleaner organization)
- ✅ **Comprehensive test suite** - `src/test_responsive_theme.cpp` (13 tests)
  - Breakpoint classification tests (480x320, 800x480, 1024x600, 1280x720)
  - Edge case tests (479, 480, 481, 800, 801 pixel boundaries)
  - Portrait/landscape rotation tests
  - Theme toggle preservation tests
- ✅ **Font size increases** - Heading (20→24), Small (10→12) for better readability
- ✅ **Breakpoint centralization** - UI_BREAKPOINT_SMALL_MAX (480), UI_BREAKPOINT_MEDIUM_MAX (800) in ui_theme.h

**Current State:**
- Patch applied and verified in LVGL source code
- Tests reveal widgets getting padding=10 instead of expected padding=12 for SMALL screens
- **Issue:** Either DPI scaling or style inheritance affecting base widget padding
- **Next:** Debug why `lv_obj_create()` padding differs from PAD_DEF macro values

**Files Modified:**
- New: `patches/lvgl_theme_breakpoints.patch`, `mk/patches.mk`, `src/test_responsive_theme.cpp`
- Modified: `mk/tests.mk`, `Makefile`, `lv_conf.h`, `main.cpp`, `globals.xml`
- Updated: `ui_theme.h`, `ui_panel_test.cpp`, `ui_wizard.cpp`, `ui_wizard_wifi.cpp`, `ui_switch.cpp`

---

## 🎯 Active Work & Next Priorities

1. **Debug Responsive Theme Test Failures** (IMMEDIATE)
   - Widgets getting padding=10, not 12 (PAD_SMALL for MEDIUM, not PAD_DEF for SMALL)
   - Investigate: DPI scaling via LV_DPX_CALC() macro
   - Check: Which style property base widgets actually use (might not be pad_all)
   - Test: Create buttons/cards vs base lv_obj to see if different widgets behave differently
   - **Goal:** Understand LVGL's actual padding behavior, adjust tests or patch accordingly

2. **Complete Responsive Theme Work** (After tests pass)
   - Remove scattered `ui_theme_register_responsive_padding()` code (now redundant with patch)
   - Simplify globals.xml padding constants (LVGL handles responsiveness now)
   - Manual test all screen sizes (480x320, 800x480, 1024x600)
   - Document patch system in BUILD_SYSTEM.md

3. **Additional Wizard Screens** (Deferred)
   - Moonraker connection, printer identification, hardware selection, summary

---

## 📋 Critical Architecture Patterns

### Pattern #0: Flex Layout Height Requirements

**When using `flex_grow` on children, parent MUST have explicit height:**

```xml
<!-- BROKEN: Parent has no height -->
<lv_obj flex_flow="row">
    <lv_obj flex_grow="3">Left (30%)</lv_obj>
    <lv_obj flex_grow="7">Right (70%)</lv_obj>
</lv_obj>
<!-- Result: Columns collapse to 0 height -->

<!-- CORRECT: Two-column pattern (30/70 split) -->
<view height="100%" flex_flow="column">
    <lv_obj width="100%" flex_grow="1" flex_flow="column">
        <lv_obj width="100%" flex_grow="1" flex_flow="row">
            <!-- BOTH columns MUST have height="100%" -->
            <lv_obj flex_grow="3" height="100%"
                    flex_flow="column" scrollable="true" scroll_dir="VER">
                <lv_obj height="100">Card 1</lv_obj>
                <lv_obj height="100">Card 2</lv_obj>
            </lv_obj>
            <lv_obj flex_grow="7" height="100%"
                    scrollable="true" scroll_dir="VER">
                <!-- Content -->
            </lv_obj>
        </lv_obj>
    </lv_obj>
</view>
```

**Critical Checks:**
1. Parent has explicit height (`height="300"`, `height="100%"`, or `flex_grow="1"`)
2. ALL columns have `height="100%"` (row height = tallest child)
3. Every level has sizing (wrapper → row → columns)
4. Cards use fixed heights (`height="100"`), NOT `LV_SIZE_CONTENT` in nested flex

**Diagnostic:** Add `style_bg_color="#ff0000"` to visualize bounds

**Reference:** `docs/LVGL9_XML_GUIDE.md:634-716`, `.claude/agents/widget-maker.md:107-149`, `.claude/agents/ui-reviewer.md:101-152`

### Pattern #1: Runtime Constants for Responsive Design & Theming

**Use case:** Single XML template that adapts to different screen sizes or theme preferences

**Example 1: Responsive dimensions**
```cpp
// C++ - Detect screen size and override constants BEFORE creating XML
int width = lv_display_get_horizontal_resolution(lv_display_get_default());
lv_xml_component_scope_t* scope = lv_xml_component_get_scope("globals");

if (width < 600) {  // TINY
    lv_xml_register_const(scope, "wizard_padding", "6");
    lv_xml_register_const(scope, "wizard_gap", "4");
} else if (width < 900) {  // SMALL
    lv_xml_register_const(scope, "wizard_padding", "12");
    lv_xml_register_const(scope, "wizard_gap", "8");
} else {  // LARGE
    lv_xml_register_const(scope, "wizard_padding", "20");
    lv_xml_register_const(scope, "wizard_gap", "12");
}

// XML - Uses runtime-modified constants
<lv_obj style_pad_all="#wizard_padding" style_pad_column="#wizard_gap">
```

**Example 2: Theme colors (light/dark mode)**
```cpp
// globals.xml - Define light/dark variants (NO hardcoded colors in C++)
<color name="app_bg_color_light" value="#F0F3F9"/>
<color name="app_bg_color_dark" value="#1F1F1F"/>

// C++ - Read from XML and override runtime constant
lv_xml_component_scope_t* scope = lv_xml_component_get_scope("globals");
const char* bg_light = lv_xml_get_const(NULL, "app_bg_color_light");
const char* bg_dark = lv_xml_get_const(NULL, "app_bg_color_dark");
lv_xml_register_const(scope, "app_bg_color", use_dark ? bg_dark : bg_light);

// Or read variant directly without override
const char* bg = use_dark ? lv_xml_get_const(NULL, "app_bg_color_dark")
                          : lv_xml_get_const(NULL, "app_bg_color_light");
```

**Why:** One XML template adapts to any screen size/theme without duplication or C++ layout manipulation

**Files:** `ui_wizard.cpp:71-124`, `ui_theme.cpp:46-80`, `main.cpp:698-709`, `globals.xml:148-164`

### Pattern #2: Navigation History Stack

**When to use:** Overlay panels (motion, temps, extrusion, keypad)

```cpp
ui_nav_push_overlay(motion_panel);  // Shows overlay, saves history
if (!ui_nav_go_back()) { /* fallback */ }
```

**Files:** `ui_nav.h:54-62`, `ui_nav.cpp:250-327`

### Pattern #3: Subject Initialization Order

**MUST initialize subjects BEFORE creating XML:**

```cpp
lv_xml_register_component_from_file("A:/ui_xml/my_panel.xml");
ui_my_panel_init_subjects();  // FIRST
lv_xml_create(screen, "my_panel", NULL);  // AFTER
```

### Pattern #4: Thread-Safety with lv_async_call() ⚠️ CRITICAL

**LVGL is NOT thread-safe.** Backend threads (WiFi, networking, file I/O) cannot create/modify widgets directly.

**Solution:** Use `lv_async_call()` to dispatch UI updates to main thread

**See ARCHITECTURE.md "Thread Safety" section for:**
- Complete code example with CallbackData struct pattern
- When to use vs. when subjects are sufficient
- Memory management best practices
- Reference implementation in `src/wifi_manager.cpp:102-190`

**Quick rule:** If backend callback needs to call ANY `lv_obj_*()` function, wrap it in `lv_async_call()`

---

## 🔧 Known Issues & Gotchas

### LVGL 9 XML Roller Options

**Issue:** XML parser fails with `options="'item1\nitem2' normal"` syntax

**Workaround:** Set roller options programmatically:
```cpp
lv_roller_set_options(roller, "Item 1\nItem 2\nItem 3", LV_ROLLER_MODE_NORMAL);
```

**Files:** `src/ui_wizard.cpp:352-387`

---

**Rule:** When work is complete, DELETE it from HANDOFF immediately. Keep this document lean and current.
