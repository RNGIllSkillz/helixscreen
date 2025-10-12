# UI Requirements - [Panel Name]

> **Purpose**: Define UI specifications for screenshot review agent
> **Version**: [Date/Version]
> **Panel**: [e.g., home_panel, controls_panel]

## Layout Requirements

### Overall Structure
- [ ] Panel dimensions: [specify width x height or percentage]
- [ ] Background color: [hex or constant name]
- [ ] Padding: [specify all sides or individual]
- [ ] Flex direction: [row/column]

### Component Positioning
- [ ] Component A positioned at: [location/alignment]
- [ ] Component B positioned at: [location/alignment]
- [ ] Spacing between components: [specify gap]

## Visual Components

### [Component Name - e.g., Navigation Bar]
- [ ] Width: [value]
- [ ] Height: [value]
- [ ] Background: [color]
- [ ] Border: [width, color, radius]
- [ ] Contains: [list child elements]

### [Component Name - e.g., Status Card]
- [ ] Dimensions: [width x height]
- [ ] Background color: [value]
- [ ] Border radius: [value]
- [ ] Padding: [value]
- [ ] Alignment: [center/left/right]

## Typography

### Primary Text
- [ ] Font: [name and size]
- [ ] Color: [hex or constant]
- [ ] Weight: [normal/bold]
- [ ] Alignment: [left/center/right]

### Secondary Text
- [ ] Font: [name and size]
- [ ] Color: [hex or constant]
- [ ] Weight: [normal/bold]
- [ ] Alignment: [left/center/right]

### Icons
- [ ] Icon font: [name and size]
- [ ] Color: [hex or constant]
- [ ] Size: [width x height]
- [ ] Spacing from label: [margin value]

## Colors

### Required Color Palette
- **Background**: `#111410` (panel_bg)
- **Card Background**: `#202020` (card_bg)
- **Primary Text**: `#ffffff` (text_primary)
- **Secondary Text**: `#909090` (text_secondary)
- **Primary Brand**: `#ff4444` (primary_color)
- **Accent**: `#00aaff` (accent_color)

### Component-Specific Colors
- [ ] [Component] uses: [color] for [purpose]
- [ ] [Component] hover state: [color]
- [ ] [Component] active state: [color]

## Spacing & Alignment

### Padding
- [ ] Panel padding: `20px` (padding_normal)
- [ ] Card padding: `20px` (padding_normal)
- [ ] Small padding: `10px` (padding_small)

### Margins
- [ ] Icon to label: `8px`
- [ ] Between cards: [specify]
- [ ] Section spacing: [specify]

### Alignment
- [ ] Main axis: [start/center/end/space-between/space-evenly]
- [ ] Cross axis: [start/center/end/stretch]

## Images & Assets

### [Asset Name - e.g., Printer Image]
- [ ] Source: [path]
- [ ] Dimensions: [width x height]
- [ ] Format: [PNG/JPG]
- [ ] Recoloring: [yes/no, if yes specify color and opacity]

## Interactive Elements

### [Element Name - e.g., Light Button]
- [ ] Clickable: [yes/no]
- [ ] Default state appearance: [describe]
- [ ] Hover state: [describe if visible]
- [ ] Active state: [describe]
- [ ] Visual feedback: [describe]

## Responsive Behavior

### Flex Growth
- [ ] [Component] flex_grow: [value]
- [ ] [Component] should take [percentage]% of space

### Wrapping
- [ ] Content wraps: [yes/no]
- [ ] Wrap direction: [row_wrap/column_wrap]

## Data Binding

### Reactive Elements
- [ ] [Element] binds to subject: [subject_name]
- [ ] Updates on: [trigger/event]
- [ ] Expected format: [string format/value range]

## Known Issues / Edge Cases

### Issues to Verify Fixed
- [FIXED] [Description of previously broken feature]
- [FIXED] [Another fixed issue]

### New Additions to Verify
- [NEW] [Description of new feature]
- [NEW] [Another new feature]

### Known Limitations
- [KNOWN] [Description of intentional limitation]

## Notes

- Add any additional context
- Special considerations
- Future enhancements to consider
- Dependencies on other components

---

## Usage

Use this template to create specific requirements for each panel review:

```bash
cp docs/templates/ui-requirements-template.md docs/requirements/home-panel-v1.md
# Edit the file with specific requirements
./scripts/review-ui-screenshot.sh \
  --screenshot /tmp/ui-screenshot-latest.png \
  --requirements docs/requirements/home-panel-v1.md \
  --xml-source ui_xml/home_panel.xml \
  --changelog docs/changelogs/home-panel-changes.md
```
