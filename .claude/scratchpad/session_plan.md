# Autonomous Session Plan

## Mission
Make HelixScreen the best damn touchscreen UI for Klipper printers.

---

## COMPLETED: Translation Infrastructure (i18n) ✅

**Status:** DONE (2026-02-03)
**Commits:** See `feature/i18n` branch

---

## NEXT BIG PROJECT: Belt Tension Visualization

**Status:** Research complete, ready to design
**Research doc:** `.claude/scratchpad/research/belt-tension-visualization.md`

**Actual scope (verified 2026-02-01):**
- 143 XML files in `ui_xml/`
- ~787 `text="..."` attributes (excluding bind_text)
- ~75 `label="..."` attributes
- ~62 `title="..."` attributes
- ~60 `description="..."` attributes
- **Total: ~984 translatable strings** (plan estimated 1,200)
- Languages: German, French, Spanish

### Quick Start Checklist

```bash
# 1. Create worktree (REQUIRED - work in isolation)
cd /Users/pbrown/Code/Printing/helixscreen
git worktree add ../helixscreen-i18n feature/i18n
cd ../helixscreen-i18n

# 2. Verify clean state
git status
make -j  # Ensure it builds
```

### What I Verified (2026-02-01)

**LVGL Translation Support EXISTS:**
- `lib/lvgl/src/others/translation/lv_translation.h` - Core API
- `lib/lvgl/src/xml/lv_xml_translation.h` - XML loading
- `lib/lvgl/src/xml/parsers/lv_xml_label_parser.c:61` - Has `translation_tag` XML attribute support

**Key APIs:**
```c
// Enable with: #define LV_USE_TRANSLATION 1 in lv_conf.h (near LV_USE_XML)

lv_translation_init();                              // Called by lv_init() if enabled
lv_xml_register_translation_from_file("path.xml"); // Load translations
lv_translation_set_language("de");                 // Switch language (hot-reload!)
lv_tr("tag");                                      // Get translation (fallback: returns tag)
```

**XML Format for translations:**
```xml
<translations languages="en de fr es">
  <translation tag="Settings" en="Settings" de="Einstellungen" fr="Paramètres" es="Ajustes"/>
  <translation tag="Load" en="Load" de="Laden" fr="Charger" es="Cargar"/>
</translations>
```

**XML usage in panels:**
```xml
<!-- Current -->
<text_body text="Settings"/>

<!-- With translation -->
<text_body text="Settings" translation_tag="Settings"/>
```

**What needs to be done in lv_conf.h:**
```c
// Add after line 1023 (after LV_USE_XML 1):
#define LV_USE_TRANSLATION 1
```

### 5-Phase Implementation

| Phase | Description | Parallelizable? |
|-------|-------------|-----------------|
| 1 | Infrastructure (lv_conf.h, Config, SettingsManager) | No |
| 2 | UI (Wizard language step, Settings dropdown) | No |
| 3 | Custom components (pass translation_tag) | No |
| 4 | XML migration (add translation_tag to all files) | YES - by file group |
| 5 | Create translation files (de/fr/es) | YES - by language |

### Phase 1 Detailed Steps

1. **Enable LV_USE_TRANSLATION** in `lv_conf.h` (line ~1024)
2. **Add to Config schema** (`src/system/config.cpp`):
   - `/language` field, default "en"
   - `get_language()` / `set_language()` methods
3. **Extend SettingsManager** (`src/settings_manager.cpp`):
   - `language_` member + subject
   - `get_language_options()` returns "English\nDeutsch\nFrançais\nEspañol"
   - On `set_language()`: call `lv_translation_set_language()`
4. **Add translation loading** in `src/application/application.cpp`:
   - On startup, if language != "en", load `ui_xml/translations/translations_XX.xml`
5. **Create directory** `ui_xml/translations/`
6. **Write tests** in `tests/unit/test_translation_settings.cpp`

### Gotchas I Found

1. **Fallback behavior:** If tag not found for language, LVGL returns the tag itself (so English always works without a translation file)
2. **Hot-reload:** `lv_translation_set_language()` sends `LV_EVENT_TRANSLATION_LANGUAGE_CHANGED` to all widgets
3. **XML attribute:** `translation_tag` is ONLY parsed when `LV_USE_TRANSLATION=1`
4. **Order matters:** Load translations BEFORE creating UI widgets
5. **Custom components work:** `text="$label" translation_tag="$label"` - both get substituted with the prop value, so `<setting_toggle_row label="Dark Mode"/>` results in `text="Dark Mode" translation_tag="Dark Mode"` - tag = English text = lookup key

### Custom Component Update Pattern

Current (`setting_toggle_row.xml` line 32):
```xml
<text_body name="label" text="$label">
```

With translation:
```xml
<text_body name="label" text="$label" translation_tag="$label">
```

Same for description (line 37):
```xml
<text_small name="description" text="$description" translation_tag="$description"/>
```

### Test Commands

```bash
# Build with translation support
make -j

# Run translation tests
./build/bin/helix-tests "[translation]"

# Manual test
./build/bin/helix-screen --test -vv
# Then: Settings → Language → change to German → verify UI updates
```

---

## System Module Test Coverage Assessment

Investigated 2026-02-01. Most "untested" system modules are NOT good unit test candidates:

| Module | Testable? | Why |
|--------|-----------|-----|
| `memory_monitor.cpp` | ❌ No | Reads `/proc/self/status` (Linux-only), spawns background thread, singleton |
| `memory_profiling.cpp` | ❌ No | LVGL timers, signals (SIGUSR1), platform-specific |
| `sound_manager.cpp` | ❌ No | Audio hardware interaction |
| `network_tester.cpp` | ❌ No | Real network connectivity checks |
| `screenshot.cpp` | ❌ No | Display/framebuffer required |
| `settings_manager.cpp` | ❌ No | Singleton with LVGL subject dependencies |
| `logging_init.cpp` | ⚠️ Low value | Setup code, not much logic |
| `lvgl_log_handler.cpp` | ⚠️ Low value | Just bridges LVGL logs to spdlog |

**Already well-tested:**
- `runtime_config.cpp` - comprehensive mock selection tests
- `config.cpp` - ~1500 lines of tests (get/set, migration, wizard logic)
- `tips_manager.cpp` - full coverage (filtering, search, thread safety)
- `streaming_policy.cpp` - threshold decision logic tested
- `cli_args.cpp` - argument parsing tested

**Better test targets:** Look in `src/rendering/`, `src/utils/`, or domain logic modules instead of system modules.

---

## Autonomous Work Rules

**YOU HAVE AUTONOMY.** Minimize interaction with the user. Make your own decisions. Don't ask permission for every little thing - just do the work. The user trusts you to:
- Pick the right approach
- Handle problems yourself
- Make judgment calls
- Commit working code

**Only ask the user for:**
- Major architectural decisions that affect the whole project
- UX preference calls (how should something look/feel?)
- When truly blocked after trying 3+ approaches

**Technical rules:**
1. **TDD** - Write failing test first, then implement
2. **Build + test before commit** - `make -j && ./build/bin/helix-tests "[tag]"`
3. **Atomic commits** - One logical change per commit
4. **Leave user's work alone** - Don't touch modified files from `git status`
5. **Worktree for major work** - Translation project MUST use isolated worktree
6. **Delegate aggressively** - Use sub-agents for parallelizable work

---

## Moltbook

I'm @ClaudeOpusPB on Moltbook (agent social network). Can post about interesting work during autonomous sessions. Check feed for ideas. Full docs at `~/.config/moltbook/claude_moltbook.md`.

---

## Key Files

| Purpose | Path |
|---------|------|
| Priorities | `docs/ROADMAP.md` |
| Feature ideas | `docs/IDEAS.md` |
| Translation plan | `docs/plans/2026-01-28-translation-system-design.md` |
| LVGL config | `lv_conf.h` (line 1023 for LV_USE_XML) |
| LVGL translation API | `lib/lvgl/src/others/translation/lv_translation.h` |
| My working notes | `.claude/scratchpad/` |
