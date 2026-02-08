# Sound System v2 — Session Continuation Prompt

Copy everything below the line into a new session.

---

## Continue: Sound System v2 — Phase 3+

I'm continuing development of the Sound System v2 for HelixScreen. Phases 1 and 2 are complete and passing all tests in a worktree. Here's the full context.

### Design Doc
Read `docs/plans/2026-02-07-sound-system-v2-design.md` for the full spec.

### Worktree
Work is in `.worktrees/sound-system-v2` on branch `feature/sound-system-v2`. Build with `make -j`, test with `make test -j && ./build/bin/helix-tests "[sound]"`.

### What's Done (Phase 1 + Phase 2) — ALL TESTS PASSING (5156 assertions, 78 test cases)

**Core engine (Phase 1):**
- `include/sound_backend.h` — Abstract interface: `set_tone(freq, amplitude, duty)`, `silence()`, `set_waveform(Waveform)`, `set_filter(type, cutoff)`, capability queries
- `include/sound_theme.h` + `src/system/sound_theme.cpp` — JSON theme parser with note name resolution (A4=440 12-TET), musical durations (4n/8n/dotted/triplet), ADSR/LFO/sweep/filter parsing, defensive defaults
- `include/sound_sequencer.h` + `src/system/sound_sequencer.cpp` — Dedicated thread, tick loop (respects backend min_tick_ms), ADSR state machine, sinusoidal LFO, linear sweep interpolation, priority preemption (UI < EVENT < ALARM). Calls set_waveform/set_filter when backend supports them, with filter sweep interpolation.
- `include/sound_manager.h` + `src/system/sound_manager.cpp` — Refactored singleton with `play(name, priority)`, theme management, M300SoundBackend (inline), auto-detection (SDL > M300), backward compat methods
- `config/sounds/default.json` — 12 sounds (button_tap, toggle_on/off, nav_forward/back, dropdown_open, print_complete, print_cancelled, error_alert, error_tone, alarm_urgent, test_beep)
- `config/sounds/minimal.json` — 4 sounds (print_complete, error_alert, error_tone, test_beep)

**SDL Backend (Phase 2):**
- `include/sdl_sound_backend.h` + `src/system/sdl_sound_backend.cpp` — Full SDL2 audio backend:
  - Waveform sample generation (square/saw/triangle/sine) via static `generate_samples()` method
  - SDL audio callback model (low-latency, no SDL_QueueAudio)
  - Atomic parameter passing from main thread to audio thread (freq, amplitude, duty, waveform)
  - Butterworth biquad filter (lowpass/highpass) with Nyquist-clamped cutoff
  - Phase continuity across calls (no clicks at buffer boundaries)
  - `supports_waveforms()`, `supports_amplitude()`, `supports_filter()` all return true
  - `initialize()` / `shutdown()` for SDL audio device lifecycle
- Auto-detection in `sound_manager.cpp`: SDL preferred over M300 on desktop builds (`#ifdef HELIX_DISPLAY_SDL`)

**Integration (Phase 1):**
- `src/print/print_completion.cpp` — Wired: plays sound on COMPLETE/ERROR/CANCELLED state transitions
- `include/settings_manager.h` + `src/system/settings_manager.cpp` — Added `ui_sounds_enabled` (bool), `sound_theme` (string) settings with persistence
- `config/helixconfig.json.template` — Updated with new settings, fixed completion_alert type (int not bool)

**Tests:**
- `tests/unit/test_sound_theme.cpp` — 28 cases: JSON parsing, note names, musical durations, ADSR/LFO/sweep/filter, defaults, error handling, clamping
- `tests/unit/test_sound_sequencer.cpp` — 20 cases: single tone, multi-step, pause, ADSR phases, LFO, sweep, priority preemption, repeat, stop, threading
- `tests/unit/test_sdl_sound_backend.cpp` — 30 cases: waveform correctness (square/saw/triangle/sine), amplitude scaling, zero amplitude silence, phase continuity, biquad filter attenuation (lowpass/highpass), filter passthrough at extreme cutoff, duty cycle effects, edge cases (very high/low frequency), capability flags

### What's Next

**Phase 3 — UI Event Hooks (sounds come alive)**
- Hook button taps via global XML event callback
- Hook nav forward/back in `ui_nav_manager.cpp`
- Hook toast errors in `ui_toast.cpp`
- Settings UI: theme picker dropdown, ui_sounds toggle row
- Settings panel XML updates

**Phase 4 — PWM Backend (real AD5M hardware)**
- `include/pwm_sound_backend.h` + `src/system/pwm_sound_backend.cpp`
- Sysfs PWM control (/sys/class/pwm/pwmchip0/pwm6/)
- Duty cycle approximation for waveforms
- Platform auto-detection

**Phase 5 — Polish**
- `retro.json` theme (chiptune vibes)
- Sound preview in settings
- M300 batch mode optimization

### Development Methodology — FOLLOW THIS EXACTLY

1. **TDD**: Write failing tests FIRST, then implement to make them pass
2. **Agent swarm**: Use `TeamCreate` to create a team, spawn `general-purpose` sub-agents for implementation work. Delegate — don't do implementation in the main session
3. **Task tracking**: Use TaskCreate/TaskUpdate to track progress with dependencies
4. **SUPER THOROUGH code reviews** after each phase: Spawn a `superpowers:code-reviewer` agent that reviews ALL new/modified files for architecture, correctness, performance, thread safety, DRY, test quality, code standards, JSON robustness. Fix all MUST-FIX issues before moving on.
5. **Worktree isolation**: All work in `.worktrees/sound-system-v2`
6. **Build verification**: `make -j && make test -j && ./build/bin/helix-tests "[sound]"` after every change

### Nice-to-Have (from earlier reviews, can address in later phases)
- Pass SoundDefinition by move or reference-to-loaded-theme to avoid string copies under mutex
- Skip inter-step silence for M300 backend (batch M300 commands)
- Add print_cancelled and alarm_urgent to minimal.json
- Document ADSR behavior when release_ms > duration_ms
- Consider marking sounds as UI/event in JSON schema instead of hardcoded is_ui_sound() list

Start with Phase 3 (UI Event Hooks). Set up the team, create tasks with dependencies, spawn agents, TDD, review.
