#!/usr/bin/env bats
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Tests for extract_release() safe rollback in scripts/lib/installer/release.sh

RELEASE_SH="scripts/lib/installer/release.sh"

setup() {
    source tests/shell/helpers.bash
    export GITHUB_REPO="prestonbrown/helixscreen"
    source "$RELEASE_SH"

    # Set up isolated test environment
    export TMP_DIR="$BATS_TEST_TMPDIR/tmp"
    export INSTALL_DIR="$BATS_TEST_TMPDIR/opt/helixscreen"
    export SUDO=""
    export BACKUP_CONFIG=""
    export ORIGINAL_INSTALL_EXISTS=""

    mkdir -p "$TMP_DIR"
}

# Helper: create a valid test tarball containing a fake ELF binary
# The tarball extracts to helixscreen/ (relative)
create_test_tarball() {
    local platform=${1:-ad5m}
    local staging="$BATS_TEST_TMPDIR/staging"
    mkdir -p "$staging/helixscreen/bin"
    mkdir -p "$staging/helixscreen/config"
    mkdir -p "$staging/helixscreen/ui_xml"

    # Create appropriate fake ELF for the platform
    case "$platform" in
        ad5m|k1|pi32)
            create_fake_arm32_elf "$staging/helixscreen/bin/helix-screen"
            ;;
        pi)
            create_fake_aarch64_elf "$staging/helixscreen/bin/helix-screen"
            ;;
    esac
    chmod +x "$staging/helixscreen/bin/helix-screen"

    # Create tarball
    tar -czf "$TMP_DIR/helixscreen.tar.gz" -C "$staging" helixscreen
    rm -rf "$staging"
}

# Helper: create a tarball with wrong architecture
create_wrong_arch_tarball() {
    local platform=${1:-ad5m}
    local staging="$BATS_TEST_TMPDIR/staging"
    mkdir -p "$staging/helixscreen/bin"

    # Create wrong arch: if platform expects ARM32, give it AARCH64
    case "$platform" in
        ad5m|k1|pi32)
            create_fake_aarch64_elf "$staging/helixscreen/bin/helix-screen"
            ;;
        pi)
            create_fake_arm32_elf "$staging/helixscreen/bin/helix-screen"
            ;;
    esac
    chmod +x "$staging/helixscreen/bin/helix-screen"

    tar -czf "$TMP_DIR/helixscreen.tar.gz" -C "$staging" helixscreen
    rm -rf "$staging"
}

# Helper: create a tarball without the binary
create_tarball_no_binary() {
    local staging="$BATS_TEST_TMPDIR/staging"
    mkdir -p "$staging/helixscreen/config"
    echo '{}' > "$staging/helixscreen/config/helixconfig.json"

    tar -czf "$TMP_DIR/helixscreen.tar.gz" -C "$staging" helixscreen
    rm -rf "$staging"
}

# Helper: set up a fake existing installation
setup_existing_install() {
    mkdir -p "$INSTALL_DIR/bin"
    mkdir -p "$INSTALL_DIR/config"
    echo "old binary" > "$INSTALL_DIR/bin/helix-screen"
    echo '{"old": true}' > "$INSTALL_DIR/config/helixconfig.json"
}

# --- Fresh install tests ---

@test "extract_release: fresh install with correct arch succeeds" {
    create_test_tarball "ad5m"
    run extract_release "ad5m"
    [ "$status" -eq 0 ]
    [ -f "$INSTALL_DIR/bin/helix-screen" ]
}

@test "extract_release: fresh install for pi with aarch64 binary succeeds" {
    create_test_tarball "pi"
    run extract_release "pi"
    [ "$status" -eq 0 ]
    [ -f "$INSTALL_DIR/bin/helix-screen" ]
}

# --- Update with existing install ---

@test "extract_release: update replaces old install" {
    setup_existing_install
    create_test_tarball "ad5m"
    extract_release "ad5m"
    [ -f "$INSTALL_DIR/bin/helix-screen" ]
    # Old install should be in .old
    [ -d "${INSTALL_DIR}.old" ]
}

@test "extract_release: config is preserved during update" {
    setup_existing_install
    create_test_tarball "ad5m"
    extract_release "ad5m"
    [ -f "$INSTALL_DIR/config/helixconfig.json" ]
    # Config should contain old content
    grep -q '"old"' "$INSTALL_DIR/config/helixconfig.json"
}

# --- Architecture mismatch with rollback ---

@test "extract_release: wrong arch preserves old install" {
    setup_existing_install
    create_wrong_arch_tarball "ad5m"
    run extract_release "ad5m"
    [ "$status" -ne 0 ]
    # Old installation should still be in place (validation happens before swap)
    [ -f "$INSTALL_DIR/bin/helix-screen" ]
    [ -f "$INSTALL_DIR/config/helixconfig.json" ]
}

@test "extract_release: wrong arch cleans up extract dir" {
    setup_existing_install
    create_wrong_arch_tarball "ad5m"
    run extract_release "ad5m"
    [ "$status" -ne 0 ]
    # Extract dir should be cleaned up
    [ ! -d "$TMP_DIR/extract" ]
}

# --- Missing binary in tarball ---

@test "extract_release: missing binary in tarball preserves old install" {
    setup_existing_install
    create_tarball_no_binary
    run extract_release "ad5m"
    [ "$status" -ne 0 ]
    # Old installation should still be intact
    [ -f "$INSTALL_DIR/bin/helix-screen" ]
}

# --- cleanup_old_install ---

@test "cleanup_old_install: removes .old directory" {
    mkdir -p "${INSTALL_DIR}.old/bin"
    echo "old" > "${INSTALL_DIR}.old/bin/helix-screen"
    cleanup_old_install
    [ ! -d "${INSTALL_DIR}.old" ]
}

@test "cleanup_old_install: no-op when .old does not exist" {
    run cleanup_old_install
    [ "$status" -eq 0 ]
}

# --- First install (no existing dir) ---

@test "extract_release: first install works with no existing dir" {
    [ ! -d "$INSTALL_DIR" ]
    create_test_tarball "ad5m"
    run extract_release "ad5m"
    [ "$status" -eq 0 ]
    [ -f "$INSTALL_DIR/bin/helix-screen" ]
    # No .old should exist
    [ ! -d "${INSTALL_DIR}.old" ]
}

# --- Legacy config migration ---

@test "extract_release: preserves legacy config location" {
    mkdir -p "$INSTALL_DIR/bin"
    echo "old" > "$INSTALL_DIR/bin/helix-screen"
    echo '{"legacy": true}' > "$INSTALL_DIR/helixconfig.json"

    create_test_tarball "ad5m"
    extract_release "ad5m"
    # Config should be migrated to new location
    [ -f "$INSTALL_DIR/config/helixconfig.json" ]
    grep -q '"legacy"' "$INSTALL_DIR/config/helixconfig.json"
}
