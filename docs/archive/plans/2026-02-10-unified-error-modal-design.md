# Unified Error Recovery Modal

## Status: IN PROGRESS (feature/unified-error-modal branch, worktree at .worktrees/unified-error-modal)

## Problem
Two separate modals for printer errors:
1. "Printer Shutdown" (`klipper_recovery_dialog.xml` / `EmergencyStopOverlay`) — klippy enters SHUTDOWN state
2. "Printer Firmware Disconnected" (`moonraker_manager.cpp` / `NOTIFY_ERROR_MODAL`) — Klipper disconnects from Moonraker

These are two layers of the same problem. During SAVE_CONFIG both fire in sequence. Users don't care about the technical distinction.

## Design
Single unified modal that adapts buttons based on connection state:
- **Connected + SHUTDOWN**: Restart Klipper, Firmware Restart, Dismiss
- **Disconnected**: Dismiss only (or spinner + "Waiting for reconnection...")
- **State transitions**: Update buttons live if connection drops while SHUTDOWN modal is showing
- **Auto-dismiss**: When klippy returns to READY (already works in recovery dialog)

## What's Done

### RecoveryReason enum (`include/ui_emergency_stop.h`)
```cpp
enum class RecoveryReason { NONE, SHUTDOWN, DISCONNECTED };
```

### Extended EmergencyStopOverlay APIs
- `show_recovery_for(RecoveryReason)` — unified entry point
- `suppress_recovery_dialog(uint32_t duration_ms)` — time-based suppression (LVGL ticks)
- `is_recovery_suppressed()` — check suppression state
- `update_recovery_dialog_content()` — updates title/message based on reason

### MoonrakerManager integration (`src/application/moonraker_manager.cpp`)
- KLIPPY_DISCONNECTED events now route through `show_recovery_for(RecoveryReason::DISCONNECTED)` instead of separate `NOTIFY_ERROR_MODAL`

### XML dialog (`ui_xml/klipper_recovery_dialog.xml`)
- Title/message nodes named (`recovery_title`, `recovery_message`) for programmatic updates
- Button callbacks updated

### Tests (`tests/unit/test_unified_recovery_dialog.cpp`, NEW file)
- Suppression timing + expiration
- Dialog visibility for SHUTDOWN and DISCONNECTED
- Deduplication (first reason wins)
- Button presence validation

### Suppression callers updated
- `src/print/print_start_enhancer.cpp` — calls both `suppress_recovery_dialog(10000)` and `suppress_disconnect_modal(10000)`
- `src/ui/ui_overlay_timelapse_install.cpp` — same dual suppression pattern
- `src/ui/ui_change_host_modal.cpp` — host change modal integration

## What's NOT Done

1. **Button state per connection** — restart buttons should disable/hide when DISCONNECTED. No ConnectionState observer in EmergencyStopOverlay yet.
2. **Live button updates** — if connection drops while dialog is showing, buttons should update dynamically. Consider XML subject binding vs imperative C++.
3. **Suppression consolidation** — currently dual checks: MoonrakerManager checks `is_disconnect_modal_suppressed()` AND recovery dialog checks `is_recovery_suppressed()`. Should be single source.
4. **Remaining caller audit** — verify ALL callers of old `suppress_disconnect_modal()` are migrated.
5. **Integration tests** — wizard/AbortManager bypass paths, moonraker event flow, connection state changes while dialog visible.

## Key Files (WIP)
| File | Status | Changes |
|------|--------|---------|
| `include/ui_emergency_stop.h` | Modified | RecoveryReason enum, new APIs, state members |
| `src/ui/ui_emergency_stop.cpp` | Modified | show_recovery_for(), content updates, suppression |
| `src/application/moonraker_manager.cpp` | Modified | Route KLIPPY_DISCONNECTED to unified dialog |
| `ui_xml/klipper_recovery_dialog.xml` | Modified | Named nodes, callback updates |
| `tests/unit/test_unified_recovery_dialog.cpp` | NEW | 157 lines, comprehensive coverage |
| `src/print/print_start_enhancer.cpp` | Modified | Dual suppression calls |
| `src/ui/ui_overlay_timelapse_install.cpp` | Modified | Dual suppression calls |
| `src/ui/ui_change_host_modal.cpp` | Modified | Host modal integration |
| `src/ui/ui_panel_settings.cpp` | Modified | Settings panel integration |
| `tests/ui_test_utils.cpp` | Modified | Test stubs for EmergencyStopOverlay |

## Architecture Notes
- All dialog creation/updates deferred via `ui_async_call()` for WebSocket thread safety
- Suppression uses LVGL ticks (not std::chrono) — consistent with LVGL timer system
- First recovery reason wins — no dialog switching mid-show
- Guards: wizard active, AbortManager shutdown, restart_in_progress_ flag
- Build verified clean on 2026-02-11 after rebase onto main
