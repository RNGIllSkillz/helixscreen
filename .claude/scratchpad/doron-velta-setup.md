# Doron Velta Test Printer Setup

## Hardware
- **Pi**: Raspberry Pi 3 Model B Rev 1.2
- **OS**: Debian Bookworm (MainsailOS), aarch64, kernel 6.12.34
- **IP**: `192.168.1.39` (user: `pbrown`)
- **Display**: Headless — no physical screen, VNC via framebuffer

## Framebuffer VNC Setup (2026-02-08)

### What was done

1. **Forced HDMI output** (headless Pi has no fb0 by default with KMS driver):
   - Added `hdmi_force_hotplug=1` to `/boot/firmware/config.txt`
   - Added `video=HDMI-A-1:800x480@60D` to `/boot/firmware/cmdline.txt`
   - Result: `/dev/fb0` at 800x480, 16bpp

2. **Built framebuffer-vncserver** from [ponty/framebuffer-vncserver](https://github.com/ponty/framebuffer-vncserver):
   - Source: `~/framebuffer-vncserver/`
   - Binary: `/usr/local/bin/framebuffer-vncserver`
   - Deps: `libvncserver-dev`

3. **Systemd service** at `/etc/systemd/system/fbvnc.service`:
   - Auto-starts on boot
   - Serves fb0 on VNC port 5900 at 30fps
   - `sudo systemctl status fbvnc.service`

### Connecting
```bash
open vnc://192.168.1.39:5900  # macOS Screen Sharing
```

### Deploying HelixScreen
```bash
# Cross-compile
make pi-docker

# Deploy (launcher + assets + binaries)
make deploy-pi-fg PI_HOST=192.168.1.39 PI_USER=pbrown

# Or deploy just the launcher and env file for quick iteration:
rsync -avz scripts/helix-launcher.sh pbrown@192.168.1.39:~/helixscreen/bin/
rsync -avz config/helixscreen.env pbrown@192.168.1.39:~/helixscreen/config/

# Run manually
ssh -tt pbrown@192.168.1.39 "cd ~/helixscreen && ./bin/helix-launcher.sh --debug --log-dest=console"

# Run in background (survives SSH disconnect)
ssh pbrown@192.168.1.39 "cd ~/helixscreen && nohup ./bin/helix-launcher.sh --debug --log-dest=console > /tmp/helix.log 2>&1 &"
```

### Backups
- `/boot/firmware/config.txt.bak` — original config
- `/boot/firmware/cmdline.txt.bak` — original cmdline

### Clock issue
Pi 3 has no RTC — clock was wrong after reboot. Fixed with `sudo date -s`.
Consider installing `ntp` or `systemd-timesyncd` for persistent fix:
```bash
sudo timedatectl set-ntp true
```

## Bugs Found & Fixed During Setup

### 1. DRM backend used instead of fbdev (edc92f19)
**Problem**: Launcher didn't set `HELIX_DISPLAY_BACKEND=fbdev`. The systemd service template had it, but that's only used when installed via the full installer. Manual/deploy launches got DRM, which broke VNC (DRM bypasses `/dev/fb0`).

**Fix**: `helix-launcher.sh` now:
- Sources `helixscreen.env` as a defaults file (env > file > hardcoded fallback)
- Defaults to `HELIX_DISPLAY_BACKEND=fbdev` on Linux when nothing else is set
- Tests: `tests/shell/test_helix_launcher.bats` (20 tests)

### 2. Touch calibration shown without touchscreen (1b6f46db)
**Problem**: `auto_detect_touch_device()` fell back to `/dev/input/event0` when no touch-capable device was found. On this Pi, event0 is the HDMI CEC remote — not a touchscreen. This made the wizard show touch calibration unnecessarily.

**Fix**: Return empty string when no touch device with ABS_X/ABS_Y capabilities is found. The caller already handles the empty case (disables pointer input gracefully).
- Moved `is_known_touchscreen_name()` to `touch_calibration.h` for testability
- Tests: 17 new assertions verifying CEC/keyboard/mouse don't match, real touchscreens do

## Testing Progress
- [x] Framebuffer created at 800x480 via forced HDMI
- [x] VNC server running on port 5900 (systemd service)
- [x] HelixScreen binary deployed (aarch64 cross-compile)
- [x] Splash screen renders via VNC
- [x] Fbdev backend used (not DRM)
- [x] App initializes, enters main loop
- [ ] Wizard completes successfully (touch cal fix needs deploy)
- [ ] Main UI renders after wizard
- [ ] Moonraker connection works (printer is real, should connect)
- [ ] Panel navigation works via VNC click-through
- [ ] NTP setup for clock persistence

## Build Notes
- **Pi cross-compile on Mac (docker)**: ~25-30 min. Slow.
- **Use THELIO.LOCAL for Pi builds**: `make pi-test` or SSH to thelio and build there. Much faster.
