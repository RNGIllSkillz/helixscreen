# Printer Images

This directory contains printer images used in the first-run configuration wizard to help users identify their printer type.

## Available Images (12 printers)

### Voron Family
- ✅ `voron-0-2-4-750x930.jpg` - Voron V0.2 (120mm³ build)
- ✅ `voron-24r2-pro-5-750x930.webp` - Voron V2.4 (CoreXY, QGL)
- ✅ `voron-trident-pro-1-750x930.webp` - Voron Trident (3Z steppers)

### Creality K-Series
- ✅ `creality-k1-2-750x930.jpg` - Creality K1 (CoreXY, multi-MCU)

### FlashForge
- ✅ `flashforge-adventurer-5m-1-750x930.webp` - FlashForge Adventurer 5M
- ✅ `flashforge-adventurer-5m-pro-2-750x930.jpg` - FlashForge Adventurer 5M Pro

### Any cubic
- ✅ `anycubic-kobra.png` - Anycubic Kobra (LeviQ ABL)
- ✅ `anycubic-vyper.png` - Anycubic Vyper (Volcano hotend)
- ✅ `anycubic-chiron.png` - Anycubic Chiron (400×400mm)

### Rat Rig
- ✅ `ratrig-vcore3.png` - Rat Rig V-Core 3 (CoreXY)
- ✅ `ratrig-vminion.png` - Rat Rig V-Minion (Compact CoreXY)

### FLSUN
- ✅ `flsun-delta.png` - FLSUN Delta (QQ-S/Super Racer/V400)

## Missing Images (Use Generic Fallback)

The following printers will use the generic Voron V2 image (`voron-24r2-pro-5-750x930.webp`) as fallback:

### Voron
- ❌ Voron V1/Legacy
- ❌ Voron Switchwire

### Creality Ender/CR Series
- ❌ Creality Ender 3
- ❌ Creality Ender 5
- ❌ Creality CR-10
- ❌ Creality CR-6 SE

### Prusa
- ❌ Prusa i3 MK3/MK3S
- ❌ Prusa MK4
- ❌ Prusa Mini/Mini+
- ❌ Prusa XL

## Image Specifications

- **Dimensions:** 750×930 pixels (standardized)
- **Format:** PNG, JPG, or WebP
- **Background:** White or transparent preferred
- **Content:** Full printer view, centered and cropped

## Adding New Images

1. Source high-quality printer image (product photos work best)
2. Resize to 750×930px with aspect ratio preservation:
   ```bash
   magick input.jpg -resize 750x930 -gravity center -extent 750x930 -background white output.png
   ```
3. Save to this directory with descriptive filename
4. Update this README
5. Update wizard integration in `src/ui_wizard.cpp`

## Future Work

- Add remaining Voron variant images (V1, Switchwire)
- Add Creality Ender/CR series images
- Add Prusa family images
- Consider adding manufacturer logos for unidentified printers
