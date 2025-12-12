#!/bin/bash

# This script MUST run AFTER the Frameworks folder is created but BEFORE code signing

echo "=== Copying Frameworks ==="

NOTTAWA_DIR="/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up"
FRAMEWORKS_DIR="$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks"

# Ensure Frameworks directory exists
mkdir -p "$FRAMEWORKS_DIR"

# Copy Sparkle framework (excluding bin directory with build tools)
if [ -d "$NOTTAWA_DIR/lib/Sparkle.framework" ]; then
    echo "Copying Sparkle.framework..."
    rsync -av --exclude="bin" "$NOTTAWA_DIR/lib/Sparkle.framework" "$FRAMEWORKS_DIR/"

    # Code sign the framework
    if [ -n "$CODE_SIGN_IDENTITY" ] && [ "$CODE_SIGN_IDENTITY" != "-" ]; then
        codesign --force --deep --sign "$CODE_SIGN_IDENTITY" "$FRAMEWORKS_DIR/Sparkle.framework"
    else
        codesign --force --deep --sign - "$FRAMEWORKS_DIR/Sparkle.framework"
    fi
    echo "Sparkle.framework copied and signed"
else
    echo "WARNING: Sparkle.framework not found at $NOTTAWA_DIR/lib/Sparkle.framework"
fi

# List what's in Frameworks to verify
echo "Frameworks directory contents:"
ls -la "$FRAMEWORKS_DIR"