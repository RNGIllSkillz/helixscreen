# Reactive Theme System - Implementation Plan

## Status: IN PROGRESS

**Worktree:** `../helixscreen-reactive-theme`
**Branch:** `reactive-theme-system`

---

## Phase 1: Core Surface & Text Styles

### Section 1.1: theme_core Style Infrastructure
- [ ] **TEST**: Write failing tests for `theme_core_get_card_style()`, `theme_core_get_dialog_style()`, `theme_core_get_text_style()`, `theme_core_get_text_muted_style()` getters
- [ ] **IMPLEMENT**: Add styles to `helix_theme_t`, init/update/preview functions, getter APIs
- [ ] **REVIEW**: Code review of theme_core changes

### Section 1.2: ui_card Shared Styles
- [ ] **TEST**: Write failing test that card bg_color updates when theme changes (currently frozen)
- [ ] **IMPLEMENT**: Replace inline styles with `lv_obj_add_style(obj, theme_core_get_card_style(), ...)`
- [ ] **REVIEW**: Code review of ui_card changes

### Section 1.3: ui_dialog Shared Styles
- [ ] **TEST**: Write failing test that dialog bg_color updates when theme changes
- [ ] **IMPLEMENT**: Replace inline styles with `theme_core_get_dialog_style()`
- [ ] **REVIEW**: Code review of ui_dialog changes

### Section 1.4: ui_text Shared Styles
- [ ] **TEST**: Write failing test that text colors update when theme changes
- [ ] **IMPLEMENT**: Add text styles to create handlers
- [ ] **REVIEW**: Code review of ui_text changes

### Phase 1 Completion
- [ ] **FULL TEST SUITE**: `make test-run` passes
- [ ] **PHASE REVIEW**: Comprehensive code review of all Phase 1 changes
- [ ] **COMMIT**: `[phase-1] Add reactive surface and text styles to theme_core`

---

## Phase 2: Icons, Status Colors, Semantic Buttons

### Section 2.1: Icon Styles in theme_core
- [ ] **TEST**: Write failing tests for icon style getters
- [ ] **IMPLEMENT**: Add icon_text_style_, icon_muted_style_, icon_primary_style_, etc.
- [ ] **REVIEW**: Code review of icon styles in theme_core

### Section 2.2: ui_icon Refactor
- [ ] **TEST**: Write failing test that icon colors update when theme changes
- [ ] **IMPLEMENT**: Refactor variants (text/muted/primary/secondary/tertiary/success/warning/danger/info), use shared styles
- [ ] **REVIEW**: Code review of ui_icon changes

### Section 2.3: Spinner & Severity Styles in theme_core
- [ ] **TEST**: Write failing tests for spinner_style and severity style getters
- [ ] **IMPLEMENT**: Add spinner_style_, severity_info/success/warning/danger_style_
- [ ] **REVIEW**: Code review of spinner/severity styles

### Section 2.4: ui_spinner Shared Style
- [ ] **TEST**: Write failing test that spinner arc color updates when theme changes
- [ ] **IMPLEMENT**: Use `theme_core_get_spinner_style()` instead of inline color
- [ ] **REVIEW**: Code review of ui_spinner changes

### Section 2.5: ui_severity_card Shared Styles
- [ ] **TEST**: Write failing test that severity card border updates when theme changes
- [ ] **IMPLEMENT**: Use shared severity styles
- [ ] **REVIEW**: Code review of ui_severity_card changes

### Section 2.6: ui_button Semantic Widget (NEW)
- [ ] **TEST**: Write tests for ui_button with variant/text/icon attrs, auto-contrast
- [ ] **IMPLEMENT**: Create ui_button.cpp/h with LV_EVENT_STYLE_CHANGED pattern
- [ ] **REVIEW**: Code review of ui_button widget

### Phase 2 Completion
- [ ] **FULL TEST SUITE**: `make test-run` passes
- [ ] **PHASE REVIEW**: Comprehensive code review of all Phase 2 changes
- [ ] **COMMIT**: `[phase-2] Add reactive icon, spinner, severity, and button styles`

---

## Phase 3: Remove Brittle Preview Code

### Section 3.1: Delete theme_manager_refresh_preview_elements
- [ ] **TEST**: Verify theme preview still works after deletion (existing tests)
- [ ] **IMPLEMENT**: Delete ~350 lines of widget lookup code
- [ ] **REVIEW**: Code review of deletion

### Section 3.2: Simplify ui_settings_display Preview Handlers
- [ ] **TEST**: Theme preview integration test
- [ ] **IMPLEMENT**: Simplify preview handlers to just call theme_core functions
- [ ] **REVIEW**: Code review of simplified handlers

### Phase 3 Completion
- [ ] **FULL TEST SUITE**: `make test-run` passes
- [ ] **PHASE REVIEW**: Comprehensive code review of all Phase 3 changes
- [ ] **COMMIT**: `[phase-3] Remove brittle preview code, rely on reactive styles`

---

## Final Completion Checklist

- [ ] All phases marked complete above
- [ ] Full test suite passes: `make test-run`
- [ ] Final comprehensive code review completed
- [ ] Branch cleanly mergeable to main
- [ ] Manual verification: dark/light toggle works, theme preview works

---

## Test Tags

Use these tags for targeted test runs:
- `[theme-core]` - theme_core style getter tests
- `[reactive-card]` - ui_card reactive style tests
- `[reactive-dialog]` - ui_dialog reactive style tests
- `[reactive-text]` - ui_text reactive style tests
- `[reactive-icon]` - ui_icon reactive style tests
- `[reactive-spinner]` - ui_spinner reactive style tests
- `[reactive-severity]` - ui_severity_card reactive style tests
- `[ui-button]` - ui_button semantic widget tests

---

## Architecture Reference

```
helix_theme_t (theme_core.c) - singleton owns ALL shared lv_style_t objects

On theme/mode change:
  theme_core_update_colors() or theme_core_preview_colors()
  -> updates ALL style objects
  -> lv_obj_report_style_change(NULL)
  -> LVGL auto-refreshes all widgets using those styles
```

**Key Principle:** DATA in C++, APPEARANCE in XML, Shared Styles connect them reactively.
