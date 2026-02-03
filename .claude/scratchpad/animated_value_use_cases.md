# AnimatedValue - When It Actually Makes Sense

The `AnimatedValue<T>` class in `include/ui/animated_value.h` is a solid implementation for animating subject-bound values, but choosing the RIGHT values to animate is critical.

## Lessons Learned

**Temperature text displays: BAD IDEA**
- Updates arrive every 100-200ms from Moonraker
- Real heating rates: 2-3°C/second max
- Per update: 0.2-0.6°C change
- Display shows INTEGER degrees
- Result: Animating "25" → "26" is imperceptible

## Good Use Cases

Values that benefit from animation have these characteristics:
1. **Large discrete jumps** (not gradual changes)
2. **Visible intermediate states** (not rounded to integers that look the same)
3. **User-triggered changes** (not continuous sensor data)

### Fan Speed (0-100%)
- User sets fan from 0% → 80%
- 80 percentage points to animate
- Clear visual feedback for user action

### Print Progress Percentage
- Updates at file progress milestones
- Could show smooth progress rather than jumps
- Note: Progress bar already uses `LV_ANIM_ON`

### Flow Rate Changes
- When user adjusts flow multiplier
- Goes from 100% → 95% (user action)

### Visual Elements (arcs, gauges)
- Arc-style temperature indicators (not text)
- Progress rings
- Any graphical representation

### Modal/Overlay Transitions
- Already handled by NavigationManager
- Slide-in/fade-out overlays

## NOT Worth Animating

- Temperature TEXT (too granular updates, rounds to same number)
- Any continuously updating sensor data with small deltas
- Values that update faster than animation can complete

## The Class Is Good - Keep It

The AnimatedValue implementation is solid:
- Retarget pattern (mid-animation value changes chase new target)
- Threshold skipping (prevents jitter)
- SettingsManager integration (respects animations_enabled)
- RAII cleanup
- Unit tested (12 tests, 35 assertions)

Just need to pick the right use case next time.
