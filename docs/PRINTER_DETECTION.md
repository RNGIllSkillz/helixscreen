# Printer Detection Heuristics

This document details the heuristics used by HelixScreen to automatically detect and identify 3D printer brands and models from Moonraker/Klipper configuration.

## Overview

HelixScreen can automatically detect printer types by analyzing data from the Moonraker API. This helps provide appropriate defaults during initial setup and can optimize the UI for specific printer characteristics.

## Detection Data Sources

### 1. Moonraker API Endpoints

**`server.info`**
- `moonraker_version` - Moonraker software version
- `components` - List of enabled Moonraker components (may include brand-specific plugins)

**`printer.info`**
- `hostname` - Printer hostname (often contains brand/model identifier)
- `software_version` - Klipper version string
- `state_message` - Current printer state

**`printer.objects.list`**
- Complete list of configured Klipper objects
- Reveals printer-specific configurations, macros, and hardware

**`printer.configfile` (if needed)**
- Full Klipper configuration
- Contains `[printer]` section with kinematics and limits
- Build volume dimensions
- MCU board identification

## Printer Type Detection Patterns

### Voron (V0, V1, V2, Trident, Legacy, Switchwire)

**High-Confidence Indicators (>=90% certainty):**
- Hostname contains: `voron`, `v0`, `v1`, `v2`, `trident`, `switchwire`, `legacy`
- Presence of `quad_gantry_level` object (V2/Trident)
- Presence of `z_thermal_adjust` object (common Voron mod)
- Multiple Z steppers: `stepper_z`, `stepper_z1`, `stepper_z2`, `stepper_z3` (V2 has 4, Trident has 3)

**Medium-Confidence Indicators (50-89% certainty):**
- MCU board: `octopus`, `spider`, `fysetc-spider`
- CoreXY kinematics with 250mm, 300mm, or 350mm build volume
- Presence of `nevermore` or `filter` fan objects
- Chamber temperature sensor
- Exhaust fan configuration
- Stealthburner toolhead macros

**Model Differentiation:**
- **V0**: 120mm³ build volume, single Z stepper, mini afterburner
- **V1**: No quad gantry leveling, CoreXY
- **V2**: 250/300/350mm³, quad gantry leveling (QGL), 4 Z steppers
- **Trident**: 250/300/350mm³, 3 Z steppers, Z-tilt instead of QGL
- **Switchwire**: Cartesian kinematics, single Z, CoreXZ
- **Legacy**: Older Voron designs, less common

### Creality (Ender, CR-series, K1-series, Sonic Pad)

**High-Confidence Indicators:**
- Hostname contains: `ender`, `cr-10`, `cr-6`, `cr-30`, `creality`, `sonic`, `k1`, `k1max`, `k1c`
- Hostname pattern: `ender3`, `ender5`, `ender3v2`, `ender3s1`
- MCU board: `creality-v4.2.2`, `creality-v4.2.7`, `cr6-se`, `GD32F303RET6`, `CR4CU220812S12`
- Multiple MCUs named `nozzle_mcu` and `leveling_mcu` (K1 series)
- Build volume: 220×220 (Ender/K1), 235×235, 300×300 (K1 Max), 400×400

**Medium-Confidence Indicators:**
- Cartesian kinematics (Ender series, not CoreXY)
- CoreXY kinematics (K1 series)
- CR-Touch or BL-Touch probe (Ender)
- Prtouch V2 leveling system with 4 pressure sensors (K1 series)
- Sprite extruder configuration (Ender 3 S1, V2)
- TMC2209 drivers with sensorless homing (K1 series)
- Serial ports `/dev/ttyS7`, `/dev/ttyS1`, `/dev/ttyS9` (K1 series)
- ADXL345 accelerometer on nozzle MCU (K1 series)

**Model Differentiation:**
- **Ender 3**: 220×220×250mm, Cartesian, no ABL stock
- **Ender 3 V2**: 220×220×250mm, Cartesian, upgraded display
- **Ender 3 S1**: 220×220×270mm, Cartesian, Sprite direct drive, CR-Touch
- **Ender 5**: 220×220×300mm, Cartesian, cube frame
- **K1**: 220×220×250mm, CoreXY, multi-MCU (GD32F303RET6 nozzle/GD32E230F8P6 leveling), Prtouch V2, sensorless homing
- **K1 Max**: 300×300×300mm, CoreXY, multi-MCU, Prtouch V2, sensorless homing
- **K1C**: 220×220×250mm, CoreXY, multi-MCU, Prtouch V2, carbon fiber rods, sensorless homing
- **CR-10**: 300×300×400mm or larger, Cartesian
- **CR-6 SE**: 235×235×250mm, Cartesian, strain gauge ABL
- **CR-30**: Belt printer (infinite Z), Cartesian

### Prusa (MK3, MK4, Mini, XL)

**High-Confidence Indicators:**
- Hostname contains: `prusa`, `mk3`, `mk4`, `mini`, `xl`, `i3`
- MCU board: `einsy`, `buddy`, `xbuddy`, `prusa-xl-buddy`
- Build volume: 250×210×210mm (MK3/MK4), 180×180×180mm (Mini)
- Trinamic drivers with specific sensorless homing config

**Medium-Confidence Indicators:**
- PINDA probe (MK3) or SuperPINDA
- Nextruder extruder (MK4)
- Specific Prusa filament sensor
- Cartesian kinematics with bed moving in Y

**Model Differentiation:**
- **i3 MK3/MK3S/MK3S+**: Einsy board, PINDA v2, 250×210mm bed
- **MK4**: Buddy board, Nextruder, Input Shaper, 250×210mm bed
- **Mini/Mini+**: Buddy board, 180×180mm bed
- **XL**: xBuddy board, up to 5 toolheads, 360×360mm bed

### Bambu Lab (X1, P1, A1)

**Note**: Bambu printers typically don't run Klipper natively. Detection only applies to modified/custom firmware.

**High-Confidence Indicators:**
- Hostname contains: `bambu`, `x1`, `p1`, `a1`, `carbon`
- Build volume: 256×256×256mm
- HMS (Humidity Monitoring System) objects
- AMS (Automatic Material System) configuration
- Lidar-based first layer scanning

**Model Differentiation:**
- **X1 Carbon**: Carbon rods, hardened nozzle, AMS capable
- **P1P/P1S**: Similar to X1 but steel rods, P1S has enclosure
- **A1/A1 Mini**: Budget models, 256mm or 180mm bed

### Anycubic (Kobra, Vyper, Chiron)

**High-Confidence Indicators:**
- Hostname contains: `anycubic`, `kobra`, `vyper`, `chiron`
- TriGorilla board (Chiron)
- LeviQ automatic leveling system (Kobra)
- Build volume: 220×220 (Kobra), 245×245 (Vyper), 400×400 (Chiron)

**Model Differentiation:**
- **Kobra**: 220×220mm, LeviQ ABL, Direct drive
- **Vyper**: 245×245mm, Auto-leveling, Volcano hotend
- **Chiron**: 400×400×450mm, Massive build volume

### Elegoo (Neptune, Mars)

**High-Confidence Indicators:**
- Hostname contains: `elegoo`, `neptune`, `saturn`
- ZNP Robin Nano board
- Build volume: 220×220 (Neptune 2), 320×320 (Neptune 3)

### Rat Rig (V-Core 3, V-Minion)

**High-Confidence Indicators:**
- Hostname contains: `ratrig`, `vcore`, `vminion`, `v-core`
- RatOS configuration files
- 3-point Z leveling for V-Core
- CoreXY kinematics
- Build volumes: 300/400/500mm

### FlashForge (Creator, Adventurer)

**High-Confidence Indicators:**
- Hostname contains: `flashforge`, `creator`, `adventurer`, `guider`
- Proprietary FlashForge MCU
- Dual extruders (Creator Pro)

### Sovol (SV01, SV06)

**High-Confidence Indicators:**
- Hostname contains: `sovol`, `sv01`, `sv06`
- Build volumes: 280×240 (SV01), 220×220 (SV06)
- Stock Marlin converted to Klipper

### Artillery (Sidewinder, Genius)

**High-Confidence Indicators:**
- Hostname contains: `artillery`, `sidewinder`, `genius`
- Build volumes: 300×300 (Sidewinder X1), 220×220 (Genius)
- Volcano hotend

### FLSUN (Q5, QQ-S, Super Racer)

**High-Confidence Indicators:**
- Hostname contains: `flsun`, `qq-s`, `super-racer`, `v400`
- Delta kinematics
- Circular build plate dimensions
- Build volumes: 200mm diameter (QQ-S), 255mm (SR), 300mm (V400)

**Model Differentiation:**
- **Q5**: 200mm delta, budget model
- **QQ-S Pro**: 255mm delta, upgraded
- **Super Racer (SR)**: 255mm delta, fast printing
- **V400**: 300mm delta, large format

### FlashForge (Creator, Adventurer, AD5M)

**High-Confidence Indicators:**
- Hostname contains: `flashforge`, `creator`, `adventurer`, `guider`, `ad5m`, `5mpro`
- MCU: Allwinner T113-S3 processor (AD5M/5M Pro)
- Build volumes: 220×220×220mm (AD5M/5M Pro), varies by model
- Klipper mod installation (xblax or Forge-X mod for AD5M)

**Medium-Confidence Indicators:**
- Dual extruders (Creator Pro)
- Enclosed build chamber (AD5M Pro)
- Proprietary FlashForge MCU (non-modded units)

**Model Differentiation:**
- **Creator Pro**: Dual extruders, enclosed, smaller build volume
- **Adventurer 3/4**: Enclosed, single extruder, removable nozzle
- **Adventurer 5M**: 220×220×220mm, enclosed, heated bed, single extruder
- **Adventurer 5M Pro**: 220×220×220mm, enclosed, heated bed, WiFi, camera
- **Guider series**: Dual extruders, industrial grade

### Generic Klipper/Moonraker

**Fallback Detection:**
- No specific brand indicators found
- Valid Klipper + Moonraker connection
- Standard object configuration
- Display as "Custom Klipper Printer"

## Detection Algorithm

### Priority Order

1. **Exact hostname match** (90-100% confidence)
   - `voron` → Voron
   - `ender3` → Creality Ender 3
   - `prusa-mk3` → Prusa i3 MK3

2. **Object signature matching** (70-90% confidence)
   - `quad_gantry_level` + 4 Z steppers → Voron V2
   - `z_tilt` + 3 Z steppers → Voron Trident
   - Delta kinematics + circular bed → FLSUN

3. **MCU board detection** (50-70% confidence)
   - `einsy` → Prusa
   - `octopus` → Likely Voron
   - `creality-v4.2.7` → Creality

4. **Build volume + kinematics** (30-50% confidence)
   - 250×210mm + Cartesian → Likely Prusa MK3
   - 220×220mm + Cartesian → Likely Ender 3
   - 300×300mm CoreXY + QGL → Likely Voron V2 300

5. **Fallback** (0% confidence)
   - Generic "Klipper Printer"

### Confidence Scoring

Each detected characteristic adds to a confidence score:
- Hostname match: +40 points
- Unique object (QGL, z_thermal_adjust): +30 points
- MCU board match: +20 points
- Build volume match: +10 points
- Kinematics match: +10 points

**Thresholds:**
- 90+ points: High confidence, auto-select
- 50-89 points: Medium confidence, suggest to user
- <50 points: Low confidence, show as option but don't pre-select

## Implementation Notes

### Data Collection

Detection happens after successful Moonraker connection:
1. Query `printer.info` for hostname
2. Query `printer.objects.list` for object names
3. Parse object names for brand-specific patterns
4. Calculate confidence score
5. Present detected type to user with confidence indicator

### User Override

Users can always override auto-detection:
- Dropdown shows detected type (if confident)
- User can select different type from full list
- User can select "Generic / Other"
- Selection is saved to config for future use

### Future Enhancements

Potential improvements for detection accuracy:
- Query `printer.configfile` for additional detail
s
- Parse specific macro names (PRINT_START, etc.)
- Detect common mod packages (Klicky, TAP, etc.)
- Community-submitted detection patterns
- Machine learning on configuration patterns

## Usage in Wizard

The wizard uses this detection as follows:

1. **Connection Step**: User enters Moonraker IP
2. **Test Connection**: Wizard connects and queries printer info
3. **Printer Identification Step**:
   - Show detected printer type with confidence
   - Allow user to enter custom printer name
   - Dropdown to select/confirm printer type
   - "Detected: Voron V2 350" or "Unable to detect - please select"
4. **Hardware Mapping Steps**: Proceed with component selection

## Configuration Storage

Detected/selected printer information is saved to config:

```json
{
  "printers": {
    "default_printer": {
      "printer_name": "My Voron",
      "printer_type": "voron_v2_350",
      "printer_type_confidence": 95,
      "detected_at": "2025-10-26T02:00:00Z",
      "moonraker_host": "192.168.1.112",
      "moonraker_port": 7125,
      ...
    }
  }
}
```

## References

- Klipper Configuration Reference: https://www.klipper3d.org/Config_Reference.html
- Moonraker API Documentation: https://moonraker.readthedocs.io/
- VoronDesign Specifications: https://docs.vorondesign.com/
- Prusa Knowledge Base: https://help.prusa3d.com/
