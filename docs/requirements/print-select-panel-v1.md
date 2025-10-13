# Print Select Panel - UI Requirements v2

**Panel Name:** Print Select Panel
**Inspiration:** Bambu Lab X1C print file browser + Modern file managers
**Status:** In Progress - Phase 4 Polish
**Date:** 2025-10-12 (Updated)

---

## Overview

The Print Select Panel displays print files in two view modes: **Card View** (visual grid with thumbnails) and **List View** (sortable table with detailed metadata). Users can toggle between views, sort by columns, and select files to print.

**Key Features:**
- **Dual view modes:** Card view (4-column grid) and List view (sortable table)
- **View toggle button:** Icon-only button to switch between card/list modes
- **Sortable columns:** Click column headers to sort (ascending/descending)
- **Empty state:** Message when no files available
- **Confirmation dialogs:** Modal confirmation for destructive actions (delete)
- **Detail view overlay:** Full-screen detail view for file inspection and print options
- **Tab navigation:** (Deferred) Internal/SD card storage source selection

---

## 1. Panel Layout Structure

### 1.1 Overall Layout
- [x] **Layout Type:** Vertical flex container
- [x] **Sections:** Two main sections (header bar + content area)
- [x] **Background:** Dark panel background (`#panel_bg`)

### 1.2 Header Bar (Top Section)
- [ ] **Height:** 56px
- [ ] **Background:** Transparent or subtle darker shade
- [ ] **Layout:** Horizontal flex, space-between alignment
- [ ] **Padding:** 16px horizontal, 8px vertical
- [ ] **Components:**
  - Left: Tab bar (Internal/SD card) - **DEFERRED to Phase 5**
  - Right: View toggle button (icon-only, card/list mode)

### 1.3 View Toggle Button
- [x] **Type:** Icon-only button (no background, no border)
- [x] **Size:** 40×40px (32px icon + 8px padding)
- [x] **Icon Font:** fa_icons_32
- [x] **Icons:**
  - When in **card mode**: Show list icon (fa-list, `\uf03a`) - "switch to list view"
  - When in **list mode**: Show grid icon (fa-th-large, `\uf009`) - "switch to card view"
- [x] **Position:** Top-right corner of panel header
- [x] **Color:** `#text_primary` (white) with hover effect
- [x] **Click:** Toggles between card and list view modes

### 1.4 Content Area (Dynamic View Container)
- [x] **Layout:** Scrollable container (view-specific layout inside)
- [x] **Scroll:** Vertical scrolling enabled
- [x] **Padding:** 16px all sides
- [x] **Content:** Either card grid OR list table based on active view mode

---

## 2. View Modes

### 2.1 Card View (Current Implementation)
- [x] **Default view mode** on first load
- [x] **Layout:** 4-column grid with row wrapping
- [x] **Cards:** 204×280px with thumbnails, filename, metadata
- [x] **Interaction:** Click card to open detail overlay
- [x] **Scroll:** Vertical scrolling through grid

### 2.2 List View (New Feature)
- [x] **Alternative view mode** toggled via view button
- [x] **Layout:** Table with sortable column headers
- [x] **Rows:** One file per row, full width
- [x] **Columns:** Filename (flex, ~510px) | Size (80px) | Modified (130px) | Print Time (90px)
  - **Removed columns:** Thumbnail, Filament, Slicer (simplified for clarity)
- [x] **Interaction:** Click row to open detail overlay (same as card click)
- [x] **Scroll:** Vertical scrolling through table rows

### 2.3 View Mode State
- [x] **Subject:** Integer subject `view_mode` (0=Card, 1=List)
- [x] **Default:** Card view (0) on first load
- [x] **Persistence:** Per-session preference (remember last view until restart)
- [x] **Toggle:** Click view toggle button to switch modes
- [x] **Reactive:** Content area hides/shows containers based on current mode

## 3. Tab Navigation (DEFERRED TO PHASE 5)

**Status:** DEFERRED - Focus on view modes and sorting first

Storage source selection (Internal vs SD card) will be implemented in a future phase. Current implementation assumes single storage source.

---

## 4. Card View - Print File Card Component

### 3.1 Card Dimensions
- [x] **Width:** Flex-based with min-width ~240px (responsive: 3-5 columns based on screen)
- [x] **Height:** Auto (based on content)
- [x] **Padding:** 12px all sides
- [x] **Border Radius:** 8px
- [x] **Background:** #3a3d42 (dark gray) or `#card_bg`

### 3.2 Card Structure (Vertical Stack)
Components in order from top to bottom:

1. **Thumbnail section** (square, no badge in v1)
2. **Filename label** (single line, truncated)
3. **Metadata row** (print time + filament weight)
4. **Optional: Chevron icon** (DEFERRED - not in v1)

### 3.3 Thumbnail Section
- [x] **Size:** Square, ~236px × 236px (based on card flex width)
- [x] **Aspect Ratio:** Crop to square (center crop)
- [x] **Border Radius:** 6px (slightly less than card radius)
- [x] **Background:** Dark gray if image missing (#3a3d42)
- [ ] **Placeholder:** Icon or text for missing thumbnails (TBD)

### 3.4 Clock Badge Overlay
**STATUS:** DEFERRED - Not included in v1 implementation

### 3.5 Filename Label
- [x] **Font:** montserrat_16
- [x] **Color:** #ffffff (white) or `#text_primary`
- [x] **Alignment:** Left
- [x] **Max Lines:** 1 (single line)
- [x] **Truncation:** "..." ellipsis for long filenames
- [x] **Margin Top:** 8px (spacing from thumbnail)

### 3.6 Metadata Row
- [x] **Layout:** Horizontal flex, flex-start with gap
- [x] **Margin Top:** 4px (spacing from filename)
- [x] **Gap:** 12px between metadata items

**Metadata Item Structure:**
- [x] Icon + text pairs, horizontal layout
- [x] Icon: 16px FontAwesome icon (fa_icons_16), color `#primary_color` (red accent)
- [x] Text: montserrat_14, color `#text_secondary` (gray)
- [x] Gap between icon and text: 4px

**Metadata Items:**
1. **Print Time:**
   - [x] Icon: Clock icon (fa-clock-o, `\uf017`)
   - [x] Text: "19m", "1h20m", "2h1m" format

2. **Filament Weight:**
   - [x] Icon: Leaf icon (fa-leaf, `\uf06c`)
   - [x] Text: "4g", "30g", "12.04g" format

### 3.7 Optional Chevron Icon
**STATUS:** DEFERRED - Not included in v1 implementation

---

## 5. List View - Sortable Table

### 5.1 Table Structure
- [ ] **Header Row:** Fixed at top, does not scroll
- [ ] **Data Rows:** Scrollable, one file per row
- [ ] **Row Height:** 52px (40px content + 12px padding)
- [ ] **Row Background:** Transparent with hover state (`#card_bg` at 50% opacity)
- [ ] **Row Border:** 1px bottom border (`#text_secondary` at 20% opacity)

### 5.2 Column Layout

| Column | Moonraker Field | Width | Alignment | Sortable | Notes |
|--------|----------------|-------|-----------|----------|-------|
| Thumbnail | `thumbnails[0]` | 40px | Center | No | Small 40×40px preview |
| Filename | `filename` | flex | Left | Yes | Primary column, truncate with ellipsis |
| Size | `size` | 80px | Right | Yes | Format as KB/MB (e.g., "1.2 MB") |
| Modified | `modified` | 120px | Left | Yes | Format as "Oct 12, 2:30 PM", **default sort** |
| Print Time | `estimated_time` | 90px | Right | Yes | Format as "1h 20m" |
| Filament | `filament_weight_total` | 70px | Right | Yes | Format as "15g" |
| Slicer | `slicer` | 100px | Left | Yes | e.g., "PrusaSlicer", truncate if needed |

**Total Width:** 40 + flex + 80 + 120 + 90 + 70 + 100 = ~500px fixed + flex filename

### 5.3 Column Headers
- [ ] **Height:** 40px
- [ ] **Background:** `#card_bg_dark` (slightly darker than panel)
- [ ] **Font:** montserrat_14, bold
- [ ] **Color:** `#text_secondary` for inactive, `#primary_color` for active sort
- [ ] **Padding:** 8px horizontal, 12px vertical
- [ ] **Border:** 1px bottom border (`#primary_color` at 50% opacity)
- [ ] **Clickable:** All columns except Thumbnail
- [ ] **Sort Indicator:** Arrow icon (fa-caret-up/fa-caret-down) next to active column text

### 5.4 Column Header Icons
- [ ] **Sort Icons:** fa_icons_16
  - Ascending: fa-caret-up (`\uf0d8`)
  - Descending: fa-caret-down (`\uf0d7`)
- [ ] **Position:** Right side of column header text
- [ ] **Color:** `#primary_color` when active, invisible when inactive

### 5.5 Data Cells
- [ ] **Font:** montserrat_14 for most columns
- [ ] **Color:** `#text_primary` for filename, `#text_secondary` for others
- [ ] **Padding:** 8px horizontal, 6px vertical
- [ ] **Alignment:** Per column specification above
- [ ] **Thumbnail Cell:** 40×40px image, centered, rounded corners (4px)

### 5.6 Row Interaction
- [ ] **Click:** Opens detail overlay (same as card click)
- [ ] **Hover:** Background changes to `#card_bg` at 50% opacity
- [ ] **Selected:** (Optional) Highlight row with `#primary_color` at 20% opacity

### 5.7 Sorting Behavior
- [ ] **Default Sort:** Modified date, descending (newest first)
- [ ] **Click Header:** Toggle between ascending/descending
- [ ] **Visual Feedback:** Arrow icon + red text color for active column
- [ ] **State:** Store sort column + direction in subjects
- [ ] **Reactive:** Re-render rows when sort changes

---

## 6. Empty State

### 6.1 Display Conditions
- [ ] Show when file list is empty (no cards/rows to display)
- [ ] Hide when at least one file exists

### 6.2 Layout
- [ ] **Container:** Centered in content area (both horizontal and vertical)
- [ ] **Background:** Transparent
- [ ] **Padding:** 40px all sides

### 6.3 Content
- [ ] **Primary Message:**
  - Text: "No files available for printing"
  - Font: montserrat_20
  - Color: `#text_primary`
  - Alignment: Center

- [ ] **Secondary Message:**
  - Text: "Upload gcode files to get started"
  - Font: montserrat_16
  - Color: `#text_secondary`
  - Alignment: Center
  - Margin-top: 12px

- [ ] **Icon:** (Optional) Large folder icon above text
  - Font: fa_icons_64
  - Icon: fa-folder-open (`\uf07c`)
  - Color: `#text_secondary` at 50% opacity
  - Margin-bottom: 20px

### 6.4 Reactive Behavior
- [ ] Use integer subject `file_count` to track number of files
- [ ] Show empty state when `file_count == 0`
- [ ] Hide empty state when `file_count > 0`

---

## 7. Confirmation Dialog

### 7.1 Purpose
Modal dialog for destructive actions requiring user confirmation (e.g., Delete file).

### 7.2 Layout Structure
- [ ] **Overlay:** Full-screen semi-transparent backdrop
  - Background: `#panel_bg` at 80% opacity
  - Z-index: Above all other content
  - Click overlay to cancel/close

- [ ] **Dialog Box:** Centered modal card
  - Width: 400px (fixed)
  - Height: Auto (based on content)
  - Background: `#card_bg`
  - Border-radius: 12px
  - Padding: 24px
  - Shadow: Subtle drop shadow for depth

### 7.3 Dialog Content
- [ ] **Title:**
  - Text: "Delete File?" (or dynamic based on action)
  - Font: montserrat_20, bold
  - Color: `#text_primary`
  - Margin-bottom: 16px

- [ ] **Message:**
  - Text: "Are you sure you want to delete [filename]? This action cannot be undone."
  - Font: montserrat_16
  - Color: `#text_secondary`
  - Line-height: 1.5
  - Margin-bottom: 24px

- [ ] **Button Row:**
  - Layout: Horizontal flex, right-aligned
  - Gap: 12px between buttons

### 7.4 Dialog Buttons
- [ ] **Cancel Button:**
  - Text: "Cancel"
  - Font: montserrat_16
  - Background: Transparent
  - Border: 1px solid `#text_secondary`
  - Color: `#text_primary`
  - Width: 100px
  - Height: 44px
  - Border-radius: 6px
  - Click: Close dialog, no action

- [ ] **Confirm Button:**
  - Text: "Delete" (or action-specific text)
  - Font: montserrat_16
  - Background: `#primary_color` (red accent)
  - Border: None
  - Color: `#text_primary`
  - Width: 100px
  - Height: 44px
  - Border-radius: 6px
  - Click: Execute action, close dialog

### 7.5 Interaction
- [ ] **Open:** Trigger via delete button click (in detail view or list row)
- [ ] **Close Methods:**
  - Click Cancel button
  - Click outside dialog (on backdrop)
  - Press Escape key (future enhancement)
- [ ] **Keyboard:** Tab navigation between buttons (future enhancement)

### 7.6 Reactive Subjects
- [ ] `dialog_visible` - Integer (0=hidden, 1=visible)
- [ ] `dialog_title` - String (dynamic title)
- [ ] `dialog_message` - String (dynamic message)
- [ ] `dialog_confirm_text` - String (dynamic confirm button text)

---

## 8. Scrolling Behavior

### 4.1 Scroll Container
- [ ] **Container:** `lv_obj` with `flex_flow="row_wrap"` for grid
- [ ] **Scrollable:** Enable vertical scrolling (`flag_scrollable="true"`)
- [ ] **Scroll Bar:** Show on scroll, auto-hide when not scrolling
- [ ] **Scroll Bar Style:** Subtle, matches theme

### 4.2 Content Layout
- [x] Cards flow left-to-right, wrap to next row
- [x] Responsive: 3-5 cards per row (based on screen width and card min-width)
- [x] Consistent gaps between cards and rows (20px)
- [x] Bottom padding to prevent last row from touching edge (16px)

**Implementation Note:**
- Use `flex_flow="row_wrap"` with card `min_width="240px"` and `flex_grow="1"`
- Cards will naturally wrap based on available space
- No media queries needed (LVGL may not support them anyway)

---

## 5. Color Palette - OUR THEME (NO NEW COLORS)

### 5.1 Existing Colors (from globals.xml) - ALL WE NEED
- [x] `#panel_bg` (0x111410) - Panel background, tab bar background
- [x] `#card_bg` (0x202020) - Card backgrounds
- [x] `#text_primary` (0xffffff) - Filename text, active tab text
- [x] `#text_secondary` (0x909090) - Inactive tab text
- [x] `#primary_color` (0xff4444) - Metadata text, metadata icons
- [x] `#accent_color` (0x00aaff) - (Available if needed)

### 5.2 UI Element → Color Mapping (DECIDED 2025-10-11)
**Backgrounds:**
- Panel background → `#panel_bg`
- Tab bar background → `#panel_bg`
- Card background → `#card_bg`

**Text:**
- Filename (primary info) → `#text_primary` (white)
- Active tab label → `#text_primary` (white)
- Inactive tab label → `#text_secondary` (gray)
- Metadata text (time, weight) → `#text_secondary` (gray)

**Icons:**
- Metadata icons (clock, leaf) → `#primary_color` (red accent)

**Result:** Zero new colors needed, using existing palette

---

## 6. Typography

### 6.1 Existing Fonts (from globals.xml)
- [ ] `montserrat_16` - Body text
- [ ] `montserrat_20` - Section headers
- [ ] `fa_icons_32` - FontAwesome icons
- [ ] `fa_icons_48` - Larger icons

### 6.2 New Fonts Needed
- [x] `fa_icons_16` - Small icons for metadata (16px FontAwesome)

**Icons to include in fa_icons_16:**
- Clock icon: `fa-clock-o` (`\uf017`)
- Leaf icon: `fa-leaf` (`\uf06c`)

---

## 7. Spacing & Dimensions

### 7.1 Global Spacing
- [ ] Tab bar height: 56px
- [ ] Content padding: 16px all sides
- [ ] Card gap (horizontal): 20px
- [ ] Card gap (vertical): 20px

### 7.2 Card Internal Spacing
- [ ] Card padding: 12px all sides
- [ ] Thumbnail to filename gap: 8px
- [ ] Filename to metadata gap: 4px
- [ ] Metadata icon to text gap: 4px
- [ ] Metadata items gap: 12px

### 7.3 Badge Positioning
- [ ] Badge offset from thumbnail edge: 8px (top-left)
- [ ] Badge diameter: 36px

**DECISION POINT:**
- Are these spacing values consistent with home panel patterns?

---

## 8. Data Binding & Reactivity

### 8.1 Subject Requirements

**Active Tab Subject:**
- [ ] Type: Integer
- [ ] Name: `active_storage_tab`
- [ ] Values: 0 (Internal), 1 (SD card)
- [ ] Used by: Tab text colors (reactive binding)

**File List Subject:**
- [ ] Type: Array of file data structures (TBD implementation)
- [ ] Name: `print_files`
- [ ] Updates: When tab changes or files refresh
- [ ] Used by: Dynamic card generation

**DECISION POINTS:**
- How do we implement dynamic list of cards in LVGL 9 XML?
- Should each card have its own subjects or use a different pattern?
- Do we generate cards in C++ and add them to XML container?

### 8.2 File Data Structure (C++)
```cpp
struct PrintFile {
    std::string filename;
    std::string thumbnail_path;  // Path to extracted thumbnail
    int print_time_minutes;      // Total print time
    float filament_grams;        // Filament weight
    bool has_thumbnail;          // Flag if thumbnail exists
};
```

---

## 9. Interactive Elements

### 9.1 Tab Clicks
- [ ] Tabs are clickable
- [ ] Click handler switches active tab
- [ ] Tab change triggers file list refresh
- [ ] Visual feedback on click (optional)

### 9.2 Card Clicks
- [x] Cards are clickable (entire card area)
- [x] Click opens detail view overlay with file details
- [ ] Visual feedback on click (optional: subtle highlight/scale)
- [ ] Long press for context menu (future)

### 9.3 Scrolling
- [ ] Touch drag for vertical scrolling
- [ ] Momentum scrolling enabled
- [ ] Scroll bar appears on interaction

**DECISION POINTS:**
- What happens when you click a card in this version?
- Do we need a separate "print" button or is card click sufficient?
- Should cards have hover/press states?

---

## 10. Print File Detail View Overlay

**Status:** IMPLEMENTED (2025-10-12)

### 10.1 Overview
Full-screen modal overlay that appears when clicking a print file card. Displays large thumbnail, file metadata, print options, and action buttons.

### 10.2 Layout Structure
- [x] **Type:** Full-screen overlay (covers entire content area)
- [x] **Sections:** 2/3 left (thumbnail) + 1/3 right (options)
- [x] **Background:** Panel background (`#panel_bg`) at 100% opacity
- [x] **Padding:** 20px horizontal, 0px vertical

### 10.3 Left Section - Thumbnail Display (2/3 width)
- [x] **Layout:** Vertical flex container with space-between
- [x] **Background:** Dark gray (`#file_card_thumbnail_bg`)
- [x] **Radius:** 0 (no rounded corners)

**Components (top to bottom):**

1. **Top Overlay - Navigation Bar**
   - [x] Height: 60px
   - [x] Background: `#panel_bg` at 50% opacity (128/255)
   - [x] Radius: 0 (no rounded corners)
   - [x] Padding: 12px horizontal
   - [x] Layout: Horizontal flex, left-aligned
   - [x] Components:
     - Back button: "&lt;" character, montserrat_28, clickable
     - Title label: "Print File", montserrat_28, primary text color

2. **Middle - Large Thumbnail**
   - [x] Size: Grows to fill available vertical space
   - [x] Alignment: Centered in container
   - [x] Source: Bound to `selected_thumbnail` subject
   - [x] Background: Dark gray if image missing

3. **Bottom Overlay - Metadata Bar**
   - [x] Height: 100px
   - [x] Background: `#panel_bg` at 50% opacity (128/255)
   - [x] Radius: 0 (no rounded corners)
   - [x] Padding: 16px horizontal (top), 0px bottom
   - [x] Layout: Vertical flex
   - [x] Components:
     - Filename: montserrat_16, white, single line with ellipsis
     - Separator: 2px horizontal line, primary color (#ff4444), full width
     - Metadata row: Horizontal flex with clock + print time, leaf + filament weight

### 10.4 Right Section - Options & Actions (1/3 width)
- [x] **Layout:** Vertical flex with space-between
- [x] **Background:** Transparent
- [x] **Gap:** 16px between elements

**Components (top to bottom):**

1. **Options Card**
   - [x] Background: `#card_bg`
   - [x] Radius: 8px
   - [x] Padding: 16px
   - [x] Layout: Vertical flex
   - [x] Flex grow: 1 (fills available space)
   - [x] Components:
     - "Filament Type" label: montserrat_16, primary text color
     - Filament dropdown: montserrat_16, dark background, 11 options (PLA, PETG, ABS, TPU, Nylon, ASA, PC, HIPS, PVA, Wood Fill, Carbon Fiber)
     - "Automatic Bed Leveling" checkbox: montserrat_16

2. **Button Row**
   - [x] Height: 50px
   - [x] Background: Transparent
   - [x] Padding: 12px top/bottom
   - [x] Gap: 12px between buttons
   - [x] Layout: Horizontal flex
   - [x] Components:
     - Delete button: Dark background (`#card_bg_dark`), montserrat_16, flex-grow: 1
     - Print button: Primary color background (`#primary_color`), montserrat_16, flex-grow: 1

### 10.5 Interactive Behavior
- [x] **Open:** Click any file card → overlay fades in, obscures grid
- [x] **Close Methods:**
   - Click back button ("<")
   - Click outside detail view (on dark background)
- [x] **Data Update:** File details update reactively via subjects
- [ ] **Delete Confirmation:** Clicking Delete shows confirmation dialog
- [ ] **Print Action:** Clicking Print starts print job

### 10.6 Reactive Data Binding
**Subjects used:**
- [x] `selected_filename` - String, filename of selected file
- [x] `selected_thumbnail` - String, path to thumbnail image
- [x] `selected_print_time` - String, formatted print time
- [x] `selected_filament_weight` - String, formatted filament weight
- [x] `detail_view_visible` - Integer, 0=hidden, 1=visible

### 10.7 Known Issues
- [ ] Thumbnail not displaying (subject bound correctly, may be image loading issue)
- [ ] Chevron-left icon needs to be added to fa_icons_32 font (currently using "<" character)
- [ ] Delete button needs confirmation dialog implementation
- [ ] Print button action not implemented

---

## 11. Edge Cases & Constraints

### 11.1 Empty States
- [ ] **No files:** Display message "No print files found"
- [ ] **Loading:** Show loading indicator while scanning files
- [ ] **SD card missing:** Show message "SD card not detected"

### 11.2 File Constraints
- [ ] Maximum files displayed: Unlimited (scrollable)
- [ ] File types supported: .gcode, .ufp (Bambu format), .bgcode
- [ ] Thumbnail extraction: From GCode metadata
- [ ] Missing thumbnail: Show placeholder icon/image

### 11.3 Performance
- [ ] Lazy loading for thumbnails (load on demand)
- [ ] Maximum cards loaded at once (virtual scrolling?)
- [ ] Thumbnail caching strategy

**DECISION POINTS:**
- Do we implement virtual scrolling or load all cards?
- How do we handle hundreds of files?
- Should we limit displayed files and add pagination/search?

---

## 11. Integration Questions

### 11.1 Navigation Integration
**Option A: Sub-panel of Home**
- Home panel has "Print Files" card that navigates to this panel
- Uses same panel area, replaces home content
- Needs back button to return to home

**Option B: Separate Top-Level Panel**
- New navigation icon in left navbar
- Becomes 6th panel in navigation
- Direct access from navbar

**DECISION POINT:** Which approach fits better with the overall UI flow?

### 11.2 File System Integration
- [ ] How do we scan for files (Moonraker API)?
- [ ] Where are thumbnails stored (extracted to temp dir)?
- [ ] How do we refresh file list (manual button or automatic)?

---

## 12. Implementation Phases

### Phase 1: Static Layout (No Data)
- [ ] Create XML structure for panel
- [ ] Implement tab bar with static tabs
- [ ] Create single static card with placeholder content
- [ ] Verify layout and styling

### Phase 2: Card Component Refinement
- [ ] Add thumbnail image support
- [ ] Implement badge overlay positioning
- [ ] Style all card elements (filename, metadata)
- [ ] Test with multiple static cards

### Phase 3: Grid & Scrolling
- [ ] Implement 4-column grid layout
- [ ] Add 12-20 static cards for scrolling test
- [ ] Enable and style scrolling behavior
- [ ] Verify spacing and alignment

### Phase 4: Reactive Tab Switching
- [ ] Create C++ wrapper with tab subject
- [ ] Add click handlers to tabs
- [ ] Implement reactive tab color updates
- [ ] Test tab switching

### Phase 5: Dynamic Card Generation
- [ ] Design approach for dynamic card creation
- [ ] Implement C++ file data structure
- [ ] Generate cards from data in C++
- [ ] Test with mock file data

### Phase 6: Real Data Integration
- [ ] Integrate with file system scanning
- [ ] Extract thumbnails from GCode metadata
- [ ] Implement thumbnail loading/caching
- [ ] Handle edge cases (missing thumbnails, empty lists)

---

## 13. Design Decisions (APPROVED 2025-10-11)

**Critical Decisions Made:**

1. ✅ **Navigation:** Separate top-level panel (6th icon in navbar)
2. ✅ **Card Width:** Flex-based responsive (min-width ~240px, adapts to 3-5 columns based on screen width)
3. ✅ **Thumbnails:** Square crop to 236×236px (uniform grid)
4. ✅ **Clock Badge:** SKIPPED in v1 (not necessary for initial implementation)
5. ✅ **Filename:** Single line with ellipsis truncation (uniform card heights)
6. ✅ **Metadata Icons:** Create fa_icons_16 font (16px FontAwesome)
7. ✅ **Filament Icon:** Leaf icon (FontAwesome fa-leaf `\uf06c`)
8. ✅ **Dynamic Cards:** XML template component + C++ instantiation (or hybrid fallback)
9. ✅ **Scrolling:** Load all cards upfront (refactor to virtual scrolling if performance issues)
10. ✅ **Card Click:** Register handler only, implement behavior in later phase

**Design Refinements:**
- Active tab visual indicator (underline/highlight)?
- Tab transition animations?
- Card hover/press visual feedback?
- Refresh button in tab bar?
- Chevron icon in v1 or defer?
- Badge border styling?
- Empty state designs?
- Loading state indicators?

---

## 14. Success Criteria

**Visual Verification:**
- [ ] Layout matches X1C reference screenshot proportions
- [ ] Cards are evenly spaced in 4-column grid
- [ ] Tabs clearly show active/inactive state
- [ ] Badge overlay positioned correctly on thumbnails
- [ ] Metadata icons and text properly aligned
- [ ] Colors match defined palette
- [ ] Scrolling is smooth with proper momentum

**Functional Verification:**
- [ ] Tab clicks switch between Internal/SD card
- [ ] Tab text color updates reactively
- [ ] Grid scrolls vertically without horizontal scroll
- [ ] All spacing matches requirements
- [ ] Cards can be clicked (handler registered)
- [ ] Panel can be navigated to and from

**Code Quality:**
- [ ] XML structure follows existing patterns
- [ ] Global constants used for all repeated values
- [ ] C++ wrapper provides clean API
- [ ] Reactive bindings work correctly
- [ ] Code is documented and maintainable

---

## 12. Moonraker API Integration

### 12.1 File Metadata Fields
Available from Moonraker's `server.files.metadata` endpoint:

**Core Fields:**
- `filename` - File name (string)
- `size` - File size in bytes (integer)
- `modified` - Last modified Unix timestamp (float)
- `estimated_time` - Print time in seconds (integer)
- `filament_weight_total` - Total filament weight in grams (float)
- `object_height` - Object height in mm (float)
- `slicer` - Slicer name (string, e.g., "PrusaSlicer")
- `slicer_version` - Slicer version (string)
- `thumbnails` - Array of thumbnail objects with `width`, `height`, `size`, `relative_path`

**Optional Fields:**
- `filament_name` - Filament brand/name
- `filament_type` - Filament type (e.g., "PLA", "PETG")
- `nozzle_diameter` - Nozzle diameter setting
- `layer_height` - Layer height setting
- `first_layer_height` - First layer height
- `print_start_time` - When print started (if file has been printed)

### 12.2 Not Available in Moonraker
- **Print count** - Not tracked by Moonraker
- **Last printed date** - Use `print_start_time` if available (only for currently/recently printed files)

For v1, we'll skip print count tracking.

---

## 13. Implementation Status

### ✅ Completed (Phase 3)

**Print File Cards:**
- ✅ 4-column grid layout (204×280px cards)
- ✅ 180×180px placeholder thumbnails
- ✅ Filename truncation with ellipsis
- ✅ Metadata row with clock + time, leaf + weight icons
- ✅ Red accent icons (#ff4444) with gray text
- ✅ 8 test cards with mock data
- ✅ Click handler for card selection

**Detail View Overlay:**
- ✅ Full-screen overlay (2/3 thumbnail + 1/3 options)
- ✅ Large scaled thumbnail (180px → 400px via C++ API)
- ✅ Semi-transparent overlays (top: back button + title, bottom: metadata)
- ✅ Back button ("<" character, chevron icon pending)
- ✅ Metadata display (filename, separator, time/filament)
- ✅ Options panel with filament type dropdown (11 types)
- ✅ Automatic bed leveling checkbox
- ✅ Delete + Print buttons (50% width each, properly centered)
- ✅ Click back or outside to close
- ✅ Reactive subject binding for metadata (filename, time, weight)

### ✅ Phase 4 - Polish (COMPLETED 2025-10-12)

**Dual View Modes:**
- ✅ Icon-only view toggle button (40×40px, fa_icons_32, no background/border)
- ✅ List icon (fa-list) shown in card mode, card icon (fa-th-large) in list mode
- ✅ View mode state management (Card=0, List=1)
- ✅ Container show/hide toggling between views

**List View with Sortable Table:**
- ✅ 4-column simplified layout: Filename (flex) | Size (80px) | Modified (130px) | Time (90px)
- ✅ Removed columns: Thumbnail, Filament, Slicer (simplified for clarity)
- ✅ Column header buttons with click handlers
- ✅ Sort indicators (▲/▼ arrows) in header labels
- ✅ Sorting by filename, size, modified date, print time
- ✅ Default sort: Filename ascending
- ✅ Toggle direction on same column, switch to ascending on new column

**Empty State:**
- ✅ Centered 400×200px container shared between card and list views
- ✅ Primary message: "No files available for printing" (montserrat_20)
- ✅ Secondary message: "Upload gcode files to get started" (montserrat_16)
- ✅ Show/hide based on file_list.empty() check

**Confirmation Dialog:**
- ✅ Reusable XML component (confirmation_dialog.xml)
- ✅ Full-screen semi-transparent backdrop
- ✅ 400×200px centered dialog card
- ✅ API properties for dynamic title and message
- ✅ Cancel + Confirm buttons (160px each)
- ✅ Click backdrop or Cancel to dismiss
- ✅ Confirm button triggers callback

**Icon System:**
- ✅ Added ICON_LIST and ICON_TH_LARGE to ui_fonts.h
- ✅ Generated #icon_list and #icon_th_large in globals.xml
- ✅ Icon constants registered in generate-icon-consts.py

**Known Limitations (Future Enhancements):**
- Thumbnail image hardcoded (not dynamically bound to subject)
- Print button has no action wired (future: Moonraker print start)
- Delete confirmation not wired to actual delete operation
- No storage tabs (Internal/SD card) - deferred to Phase 5
- No search/filter functionality - future enhancement
- No hover effects on list rows (SDL2 limitation)

**File References:**
- `ui_xml/print_select_panel.xml` - Panel with dual view containers + toggle
- `ui_xml/print_file_card.xml` - Card component
- `ui_xml/print_file_list_row.xml` - List row component (4 columns)
- `ui_xml/print_file_detail.xml` - Detail view overlay
- `ui_xml/confirmation_dialog.xml` - Reusable dialog component
- `src/ui_panel_print_select.cpp` - Business logic (619 lines)
- `include/ui_panel_print_select.h` - API definitions
- `src/ui_utils.cpp` - Formatting utilities (file size, date)

---

**Requirements Version:** 2.0
**Last Updated:** 2025-10-12 Evening
**Status:** Phase 4 COMPLETE - List View, Sorting, Empty State, Confirmation Dialogs implemented

