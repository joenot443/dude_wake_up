#!/bin/bash

# Script to copy Sparkle framework to app bundle
# Add this as a "Run Script" Build Phase in Xcode

# Get the source and destination paths
SOURCE_FRAMEWORK="${PROJECT_DIR}/lib/Sparkle.framework"
DEST_FRAMEWORKS="${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}"

# Check if Sparkle framework exists in lib
if [ ! -d "$SOURCE_FRAMEWORK" ]; then
    echo "error: Sparkle.framework not found at $SOURCE_FRAMEWORK"
    exit 1
fi

# Create Frameworks directory if it doesn't exist
mkdir -p "$DEST_FRAMEWORKS"

# Copy Sparkle framework to app bundle
echo "Copying Sparkle.framework to app bundle..."
cp -R "$SOURCE_FRAMEWORK" "$DEST_FRAMEWORKS/"

# Remove bin directory if it exists (it's not needed in the app bundle)
rm -rf "$DEST_FRAMEWORKS/Sparkle.framework/bin"

# Code sign the framework
if [ -n "$CODESIGNING_FOLDER_PATH" ]; then
    echo "Code signing Sparkle.framework..."
    codesign --force --deep --sign "$EXPANDED_CODE_SIGN_IDENTITY" "$DEST_FRAMEWORKS/Sparkle.framework"
fi

echo "Sparkle.framework successfully embedded"