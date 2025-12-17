// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "ui_observer_guard.h"

namespace helix {

/**
 * @brief Initialize auto-navigation to print status panel on print start
 *
 * Registers an observer on PrinterState's print_state_enum subject that
 * automatically navigates to the print status panel when a print starts,
 * but ONLY if the user is currently viewing the home panel.
 *
 * This handles the case where a print is started externally (via phone,
 * web interface, etc.) and the user is on the home screen waiting.
 *
 * @return ObserverGuard that manages the observer's lifetime
 */
ObserverGuard init_print_start_navigation_observer();

} // namespace helix
