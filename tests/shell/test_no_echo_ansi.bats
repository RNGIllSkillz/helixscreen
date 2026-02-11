#!/usr/bin/env bats
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Lint: ensure installer/uninstaller shell scripts use printf '%b\n' instead
# of echo for ANSI color output.  BusyBox echo doesn't interpret \033 escapes,
# so echo "${CYAN}..." prints literal garbage on AD5M/K1 targets.
#
# Checks the source modules (scripts/lib/installer/*.sh) and the non-bundled
# entry points (install-dev.sh).  Bundled scripts (install.sh, uninstall.sh)
# are auto-generated and inherit the fix from the modules.

WORKTREE_ROOT="$(cd "$BATS_TEST_DIRNAME/../.." && pwd)"

# Pattern: echo with any of the color variables we define in common.sh
# Matches: echo "${CYAN}...", echo "$RED...", echo "${BOLD}${GREEN}..."
# Allows:  echo "" (no color), echo "plain text", printf '%b\n' "${CYAN}..."
ANSI_ECHO_PATTERN='echo[[:space:]].*\$\{?(RED|GREEN|YELLOW|CYAN|BOLD|NC)[}\"]'

@test "installer modules: no echo with ANSI color variables" {
    local violations=""
    for f in "$WORKTREE_ROOT"/scripts/lib/installer/*.sh; do
        local hits
        hits=$(grep -nE "$ANSI_ECHO_PATTERN" "$f" 2>/dev/null || true)
        if [ -n "$hits" ]; then
            violations="${violations}${f##*/}:
${hits}
"
        fi
    done
    if [ -n "$violations" ]; then
        echo "Use printf '%b\\n' instead of echo for ANSI escapes:" >&2
        echo "$violations" >&2
        false
    fi
}

@test "install-dev.sh: no echo with ANSI color variables" {
    local hits
    hits=$(grep -nE "$ANSI_ECHO_PATTERN" "$WORKTREE_ROOT/scripts/install-dev.sh" 2>/dev/null || true)
    if [ -n "$hits" ]; then
        echo "Use printf '%b\\n' instead of echo for ANSI escapes:" >&2
        echo "$hits" >&2
        false
    fi
}
