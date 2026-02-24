#!/bin/bash
# run_ui_tests.sh — Build and run NottawaApp UI tests
#
# PREREQUISITES (one-time setup):
#   macOS UI tests require Accessibility permissions for your terminal app.
#   Go to: System Settings > Privacy & Security > Accessibility
#   Add your terminal app (Terminal.app, iTerm2, etc.)
#
# Usage:
#   ./run_ui_tests.sh              # Build and run all UI tests
#   ./run_ui_tests.sh --build-only # Build only, skip test execution
#   ./run_ui_tests.sh --test-only  # Run tests without rebuilding (must have built first)
#   ./run_ui_tests.sh --only-testing NottawaAppUITests/NottawaAppUITests/testSidebarVisibleOnLaunch
#                                  # Run a specific test

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BIN_DIR="$SCRIPT_DIR/bin"
XCTESTRUN="$BIN_DIR/NottawaAppUITests.xctestrun"
DESTINATION="platform=macOS,arch=arm64"

BUILD_ONLY=false
TEST_ONLY=false
ONLY_TESTING=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-only)
            BUILD_ONLY=true
            shift
            ;;
        --test-only)
            TEST_ONLY=true
            shift
            ;;
        --only-testing)
            ONLY_TESTING="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--build-only] [--test-only] [--only-testing TEST_ID]"
            exit 1
            ;;
    esac
done

# Step 1: Build
if [ "$TEST_ONLY" = false ]; then
    echo "==> Building NottawaApp and test runner..."
    xcodebuild build \
        -scheme NottawaAppUITests \
        -destination "$DESTINATION" \
        -configuration Debug \
        | tail -5

    if [ $? -ne 0 ]; then
        echo "ERROR: Build failed."
        exit 1
    fi
    echo "==> Build succeeded."
fi

if [ "$BUILD_ONLY" = true ]; then
    echo "==> Build-only mode, skipping test execution."
    exit 0
fi

# Step 2: Verify test runner exists
if [ ! -d "$BIN_DIR/NottawaAppUITests-Runner.app" ]; then
    echo "ERROR: Test runner not found at $BIN_DIR/NottawaAppUITests-Runner.app"
    echo "Run without --test-only to build first."
    exit 1
fi

# Step 3: Verify xctestrun file exists
if [ ! -f "$XCTESTRUN" ]; then
    echo "ERROR: xctestrun file not found at $XCTESTRUN"
    echo "This file should be committed to the repo."
    exit 1
fi

# Step 4: Run tests using xctestrun (bypasses build planner bug)
echo "==> Running UI tests..."
EXTRA_ARGS=""
if [ -n "$ONLY_TESTING" ]; then
    EXTRA_ARGS="-only-testing:$ONLY_TESTING"
fi

xcodebuild test-without-building \
    -xctestrun "$XCTESTRUN" \
    -destination "$DESTINATION" \
    $EXTRA_ARGS

TEST_EXIT=$?

if [ $TEST_EXIT -eq 0 ]; then
    echo "==> All UI tests passed!"
else
    echo "==> Some UI tests failed (exit code: $TEST_EXIT)"
    echo ""
    echo "If you see 'Timed out while enabling automation mode', your terminal needs"
    echo "Accessibility permissions: System Settings > Privacy & Security > Accessibility"
fi

exit $TEST_EXIT
