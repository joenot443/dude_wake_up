#!/bin/bash
set -e  # Exit on any error

echo "=== Sparkle Framework Copy Script ==="

SOURCE="/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up/lib/Sparkle.framework"
DEST="$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks"

echo "Source: $SOURCE"
echo "Destination: $DEST"

# Check source exists
if [ ! -d "$SOURCE" ]; then
    echo "ERROR: Source framework not found at $SOURCE"
    exit 1
fi

# Create destination
echo "Creating Frameworks directory..."
mkdir -p "$DEST"

# Remove old framework if exists
if [ -d "$DEST/Sparkle.framework" ]; then
    echo "Removing old Sparkle.framework..."
    rm -rf "$DEST/Sparkle.framework"
fi

# Copy framework
echo "Copying Sparkle.framework..."
cp -R "$SOURCE" "$DEST/" || { echo "ERROR: Copy failed"; exit 1; }

# Verify copy worked
if [ ! -d "$DEST/Sparkle.framework" ]; then
    echo "ERROR: Framework not copied successfully"
    exit 1
fi

# Remove bin directory
echo "Removing bin directory..."
rm -rf "$DEST/Sparkle.framework/bin"

# List contents to verify
echo "Framework contents:"
ls -la "$DEST/Sparkle.framework/" | head -5

# Sign framework
echo "Signing framework..."
codesign --force --sign - "$DEST/Sparkle.framework" || { echo "WARNING: Signing failed but continuing"; }

echo "=== Sparkle Framework Successfully Installed ==="