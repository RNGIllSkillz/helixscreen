# UI Changelog - [Panel Name]

> **Purpose**: Document changes for screenshot review agent verification
> **Version**: [Date/Version]
> **Panel**: [e.g., home_panel, controls_panel]

## Summary

**Total Changes**: X
- **Fixed Issues**: Y
- **New Additions**: Z
- **Improvements**: W

---

## Fixed Issues

### 1. [Issue Title - e.g., Network Icon Misalignment]

**Problem**:
[Describe the original problem clearly]

**Cause**:
[Technical explanation of what was wrong]

**Fix Applied**:
```xml
<!-- BEFORE (home_panel.xml:32) -->
<lv_obj flex_flow="column" style_flex_cross_place="start">
    <lv_label name="network_icon" text="#icon_wifi"/>
    <lv_label name="network_label" text="Wi-Fi"/>
</lv_obj>

<!-- AFTER -->
<lv_obj flex_flow="column" style_flex_cross_place="center">
    <lv_label name="network_icon" text="#icon_wifi"/>
    <lv_label name="network_label" text="Wi-Fi"/>
</lv_obj>
```

**Expected Visual Result**:
[Describe what should be visible in the screenshot now]
- Icon should be centered horizontally in its container
- Label should align center under icon

**Verification Criteria**:
- [ ] Icon is centered (not left-aligned)
- [ ] Label is centered (not left-aligned)
- [ ] Both elements aligned on center axis

---

### 2. [Another Fixed Issue - e.g., Light Icon Color Too Bright]

**Problem**:
[Description]

**Cause**:
[Technical explanation]

**Fix Applied**:
```xml
<!-- BEFORE -->
[XML snippet]

<!-- AFTER -->
[XML snippet]
```

**Expected Visual Result**:
[What to look for]

**Verification Criteria**:
- [ ] [Specific check]
- [ ] [Another check]

---

## New Additions

### 1. [Feature Name - e.g., Vertical Dividers in Status Card]

**Purpose**:
[Why this was added]

**Implementation**:
```xml
<!-- NEW CODE (home_panel.xml:29) -->
<lv_obj width="1"
        style_align_self="stretch"
        style_bg_color="#text_secondary"
        style_bg_opa="50%"
        style_border_width="0"
        style_pad_top="8"
        style_pad_bottom="8"/>
```

**Expected Visual Result**:
[What should be visible]
- Thin vertical line (1px wide)
- Gray color (#909090 at 50% opacity)
- Stretches full height of container
- Positioned between status sections

**Verification Criteria**:
- [ ] Divider is visible
- [ ] Divider is 1px wide
- [ ] Divider spans full height
- [ ] Color is light gray
- [ ] Separates temperature and network sections

---

### 2. [Another New Feature]

**Purpose**:
[Why added]

**Implementation**:
```xml
[XML code]
```

**Expected Visual Result**:
[Description]

**Verification Criteria**:
- [ ] [Check]

---

## Improvements

### 1. [Improvement Name - e.g., Reduced Icon Size for Better Balance]

**Rationale**:
[Why this change was made]

**Change Applied**:
```xml
<!-- BEFORE (home_panel.xml:24) -->
<lv_label text="#icon_temperature" style_text_font="fa_icons_64"/>

<!-- AFTER -->
<lv_label text="#icon_temperature" style_text_font="fa_icons_48"/>
```

**Expected Visual Result**:
[What to look for]
- Temperature icon appears smaller
- Better proportion with surrounding elements
- Matches network/light icon sizes

**Verification Criteria**:
- [ ] Icon is 48px (visually smaller than before)
- [ ] Icon proportional to other icons
- [ ] Icon doesn't overpower the temperature text

---

## Configuration Changes

### Global Constants Updated
[If any constants in globals.xml changed]

```xml
<!-- BEFORE -->
<px name="some_constant" value="100"/>

<!-- AFTER -->
<px name="some_constant" value="120"/>
```

**Affected Components**:
- [List components using this constant]

---

## Refactoring

### [Refactor Name - e.g., Consolidated Card Styling]

**What Changed**:
[Describe structural changes]

**Why**:
[Rationale for refactor]

**Impact**:
[Should not affect visual appearance, but note any subtle changes]

---

## Known Issues Not Addressed

### [Issue Name]
**Status**: Not yet fixed
**Reason**: [Why not fixed in this version]
**Plan**: [When/how will be addressed]

---

## Testing Notes

### Manual Testing Checklist
- [ ] Built successfully
- [ ] No XML parsing errors
- [ ] All subjects initialized
- [ ] Screenshot captured
- [ ] Screenshot compared to previous version

### Visual Regression
- [ ] No unintended changes to other panels
- [ ] Colors match theme
- [ ] Fonts render correctly
- [ ] No layout shifts

---

## Related Changes

### Files Modified
- `ui_xml/[panel_name].xml` - [description of changes]
- `ui_xml/globals.xml` - [if constants changed]
- `src/ui_panel_[name].cpp` - [if C++ logic changed]

### Commits
- `[commit hash]` - [commit message]

---

## Notes for Reviewer Agent

### Critical Changes
[Highlight the most important changes that MUST be verified]

1. [Critical change 1]
2. [Critical change 2]

### Subtle Changes
[Changes that might be easy to miss]

1. [Subtle change 1]
2. [Subtle change 2]

### Context
[Any additional context the agent should know]

---

## Usage

Create a changelog for each review session:

```bash
cp docs/templates/ui-changelog-template.md docs/changelogs/home-panel-2025-01-11.md
# Document your changes
./scripts/review-ui-screenshot.sh \
  --screenshot /tmp/ui-screenshot-latest.png \
  --requirements docs/requirements/home-panel-v1.md \
  --xml-source ui_xml/home_panel.xml \
  --changelog docs/changelogs/home-panel-2025-01-11.md
```
