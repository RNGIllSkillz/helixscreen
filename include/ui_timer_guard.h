// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <lvgl.h>

namespace helix::ui {

/**
 * @brief RAII wrapper for LVGL timers - automatically deletes timer on destruction
 *
 * Handles the edge case where LVGL may be deinitialized before the timer owner,
 * preventing crashes during shutdown.
 */
class LvglTimerGuard {
  public:
    LvglTimerGuard() = default;
    explicit LvglTimerGuard(lv_timer_t* timer) : timer_(timer) {}

    ~LvglTimerGuard() {
        reset();
    }

    // Move-only (no copies)
    LvglTimerGuard(const LvglTimerGuard&) = delete;
    LvglTimerGuard& operator=(const LvglTimerGuard&) = delete;

    LvglTimerGuard(LvglTimerGuard&& other) noexcept : timer_(other.timer_) {
        other.timer_ = nullptr;
    }

    LvglTimerGuard& operator=(LvglTimerGuard&& other) noexcept {
        if (this != &other) {
            reset();
            timer_ = other.timer_;
            other.timer_ = nullptr;
        }
        return *this;
    }

    void reset(lv_timer_t* timer = nullptr) {
        if (timer_ && lv_is_initialized()) {
            lv_timer_delete(timer_);
        }
        timer_ = timer;
    }

    lv_timer_t* get() const {
        return timer_;
    }
    lv_timer_t* release() {
        lv_timer_t* t = timer_;
        timer_ = nullptr;
        return t;
    }

    explicit operator bool() const {
        return timer_ != nullptr;
    }

  private:
    lv_timer_t* timer_ = nullptr;
};

} // namespace helix::ui
