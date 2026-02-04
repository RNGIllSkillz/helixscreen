#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Module: competing_uis
# Stop competing screen UIs (GuppyScreen, KlipperScreen, Xorg, stock FlashForge UI)
#
# Reads: INIT_SYSTEM, PREVIOUS_UI_SCRIPT, SUDO
# Writes: (none)

# Source guard
[ -n "${_HELIX_COMPETING_UIS_SOURCED:-}" ] && return 0
_HELIX_COMPETING_UIS_SOURCED=1

# Known competing screen UIs to stop
# Includes: GuppyScreen (AD5M/K1), Grumpyscreen (K1/Simple AF), KlipperScreen, FeatherScreen
COMPETING_UIS="guppyscreen GuppyScreen grumpyscreen Grumpyscreen KlipperScreen klipperscreen featherscreen FeatherScreen"

# Stop competing screen UIs (GuppyScreen, KlipperScreen, Xorg, etc.)
stop_competing_uis() {
    log_info "Checking for competing screen UIs..."

    found_any=false

    # Stop stock FlashForge firmware UI (AD5M/Adventurer 5M)
    # ffstartup-arm is the startup manager that launches firmwareExe (the stock Qt UI)
    if [ -f /opt/PROGRAM/ffstartup-arm ]; then
        log_info "Stopping stock FlashForge UI..."
        kill_process_by_name firmwareExe ffstartup-arm || true  # Don't fail if processes not running
        found_any=true
    fi

    # On Klipper Mod, stop Xorg first (required for framebuffer access)
    # Xorg takes over /dev/fb0 layer, preventing direct framebuffer rendering
    if [ -x "/etc/init.d/S40xorg" ]; then
        log_info "Stopping Xorg (Klipper Mod display server)..."
        $SUDO /etc/init.d/S40xorg stop 2>/dev/null || true
        # Disable Xorg init script (non-destructive, reversible)
        $SUDO chmod -x /etc/init.d/S40xorg 2>/dev/null || true
        # Kill any remaining Xorg processes
        kill_process_by_name Xorg X || true  # Don't fail if processes not running
        found_any=true
    fi

    # First, handle the specific previous UI if we know it (for clean reversibility)
    if [ -n "$PREVIOUS_UI_SCRIPT" ] && [ -x "$PREVIOUS_UI_SCRIPT" ] 2>/dev/null; then
        log_info "Stopping previous UI: $PREVIOUS_UI_SCRIPT"
        $SUDO "$PREVIOUS_UI_SCRIPT" stop 2>/dev/null || true
        # Disable by removing execute permission (non-destructive, reversible)
        $SUDO chmod -x "$PREVIOUS_UI_SCRIPT" 2>/dev/null || true
        found_any=true
    fi

    for ui in $COMPETING_UIS; do
        # Check systemd services
        if [ "$INIT_SYSTEM" = "systemd" ]; then
            if $SUDO systemctl is-active --quiet "$ui" 2>/dev/null; then
                log_info "Stopping $ui (systemd service)..."
                $SUDO systemctl stop "$ui" 2>/dev/null || true
                $SUDO systemctl disable "$ui" 2>/dev/null || true
                found_any=true
            fi
        fi

        # Check SysV init scripts (various locations)
        for initscript in /etc/init.d/S*${ui}* /etc/init.d/${ui}* /opt/config/mod/.root/S*${ui}*; do
            # Skip if glob didn't match any files (literal pattern returned)
            [ -e "$initscript" ] || continue
            # Skip if this is the PREVIOUS_UI_SCRIPT we already handled
            if [ "$initscript" = "$PREVIOUS_UI_SCRIPT" ]; then
                continue
            fi
            if [ -x "$initscript" ]; then
                log_info "Stopping $ui ($initscript)..."
                $SUDO "$initscript" stop 2>/dev/null || true
                # Disable by removing execute permission (non-destructive)
                $SUDO chmod -x "$initscript" 2>/dev/null || true
                found_any=true
            fi
        done

        # Kill any remaining processes by name
        if kill_process_by_name "$ui"; then
            log_info "Killed remaining $ui processes"
            found_any=true
        fi
    done

    # Also kill python processes running KlipperScreen (common on Klipper Mod)
    # BusyBox ps doesn't support 'aux', use portable approach
    # shellcheck disable=SC2009
    for pid in $(ps -ef 2>/dev/null | grep -E 'KlipperScreen.*screen\.py' | grep -v grep | awk '{print $2}'); do
        log_info "Killing KlipperScreen python process (PID $pid)..."
        $SUDO kill "$pid" 2>/dev/null || true
        found_any=true
    done

    if [ "$found_any" = true ]; then
        log_info "Waiting for competing UIs to stop..."
        sleep 2
    else
        log_info "No competing UIs found"
    fi
}
