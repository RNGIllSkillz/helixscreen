// Copyright 2025 HelixScreen
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file terminal_raw.h
 * @brief Terminal raw mode utilities for interactive TUI
 *
 * Based on btop++ approach - raw termios manipulation without external libraries.
 * Provides:
 * - Raw mode enable/disable (for immediate key capture)
 * - Non-blocking keyboard input
 * - Screen clearing and cursor positioning
 */

#pragma once

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>

namespace terminal {

class RawMode {
public:
    RawMode() : enabled_(false) {}

    ~RawMode() {
        if (enabled_) {
            disable();
        }
    }

    bool enable() {
        if (enabled_) return true;

        // Get current terminal settings
        if (tcgetattr(STDIN_FILENO, &orig_termios_) == -1) {
            return false;
        }

        // Set up raw mode
        struct termios raw = orig_termios_;
        raw.c_lflag &= ~(ECHO | ICANON);  // Disable echo and canonical mode
        raw.c_cc[VMIN] = 0;                // Non-blocking read
        raw.c_cc[VTIME] = 0;               // No timeout

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
            return false;
        }

        // Set stdin to non-blocking
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        enabled_ = true;
        return true;
    }

    void disable() {
        if (!enabled_) return;

        // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios_);

        // Restore blocking mode
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

        enabled_ = false;
    }

    // Read a single key (non-blocking)
    // Returns: key character, or 0 if no key available
    char read_key() {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            // Handle escape sequences (arrow keys, etc.)
            if (c == '\033') {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) != 1) return c;
                if (read(STDIN_FILENO, &seq[1], 1) != 1) return c;

                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': return 'A'; // Up arrow
                        case 'B': return 'B'; // Down arrow
                        case 'C': return 'C'; // Right arrow
                        case 'D': return 'D'; // Left arrow
                    }
                }
            }
            return c;
        }
        return 0;
    }

private:
    bool enabled_;
    struct termios orig_termios_;
};

// ANSI escape codes for screen manipulation
namespace ansi {
    inline void clear_screen() {
        std::cout << "\033[2J\033[H";
    }

    inline void move_cursor(int row, int col) {
        std::cout << "\033[" << row << ";" << col << "H";
    }

    inline void hide_cursor() {
        std::cout << "\033[?25l";
    }

    inline void show_cursor() {
        std::cout << "\033[?25h";
    }

    inline void save_cursor() {
        std::cout << "\033[s";
    }

    inline void restore_cursor() {
        std::cout << "\033[u";
    }
}

} // namespace terminal
