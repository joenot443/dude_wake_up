#!/bin/bash

# Nottawa App Store Release Script
# Handles: build archive, export, and upload to App Store Connect
#
# Usage:
#   ./release-appstore.sh VERSION           - Full release (archive + upload)
#   ./release-appstore.sh VERSION --test    - Archive only, no upload
#
# Prerequisites:
#   1. Apple Distribution certificate in keychain
#   2. App Store provisioning profile installed
#   3. App Store Connect API key OR stored credentials
#
# First time setup:
#   xcrun notarytool store-credentials "AC_PASSWORD"
#
# Example:
#   ./release-appstore.sh 1.0.2

set -e

VERSION=$1
BUILD=$2
MODE=$3
SCHEME="dude_wake_up Release"
PROJECT="dude_wake_up.xcodeproj"
ARCHIVE_PATH="build/Nottawa-${VERSION}.xcarchive"
EXPORT_PATH="build/AppStore"

if [ -z "$VERSION" ] || [ -z "$BUILD" ]; then
    echo "Usage: ./release-appstore.sh VERSION BUILD [--test]"
    echo ""
    echo "  VERSION  - Semantic version (e.g., 1.0.2)"
    echo "  BUILD    - Build number (must be higher than previous upload)"
    echo ""
    echo "Examples:"
    echo "  ./release-appstore.sh 1.0.2 30           # Full release to App Store"
    echo "  ./release-appstore.sh 1.0.2 30 --test    # Archive only, no upload"
    echo ""
    echo "Note: Last uploaded build was 31. Use 32 or higher."
    exit 1
fi

echo "=== Nottawa App Store Release v${VERSION} (Build ${BUILD}) ==="

# Step 1: Clean previous builds
echo ""
echo "[1/4] Cleaning previous builds..."
rm -rf build/Build
rm -rf "$ARCHIVE_PATH"
rm -rf "$EXPORT_PATH"

# Step 2: Create archive
echo ""
echo "[2/4] Creating archive..."
xcodebuild archive \
    -project "$PROJECT" \
    -scheme "$SCHEME" \
    -configuration Release \
    -archivePath "$ARCHIVE_PATH" \
    CODE_SIGN_STYLE=Automatic \
    CODE_SIGN_IDENTITY=- \
    DEVELOPMENT_TEAM=7VUCELU5PL \
    ARCHS="arm64 x86_64" \
    ONLY_ACTIVE_ARCH=NO

if [ ! -d "$ARCHIVE_PATH" ]; then
    echo "Error: Archive failed. Archive not found at $ARCHIVE_PATH"
    exit 1
fi

# Update version in archive
echo "Updating version to $VERSION (build $BUILD)..."
/usr/libexec/PlistBuddy -c "Set :ApplicationProperties:CFBundleShortVersionString $VERSION" "$ARCHIVE_PATH/Info.plist"
/usr/libexec/PlistBuddy -c "Set :ApplicationProperties:CFBundleVersion $BUILD" "$ARCHIVE_PATH/Info.plist"

APP_IN_ARCHIVE=$(find "$ARCHIVE_PATH/Products/Applications" -name "*.app" -type d | head -1)
if [ -n "$APP_IN_ARCHIVE" ]; then
    /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $VERSION" "$APP_IN_ARCHIVE/Contents/Info.plist"
    /usr/libexec/PlistBuddy -c "Set :CFBundleVersion $BUILD" "$APP_IN_ARCHIVE/Contents/Info.plist"

    # Remove Sparkle framework (App Store handles updates, Sparkle causes entitlement warnings)
    echo "Removing Sparkle framework (not needed for App Store)..."
    rm -rf "$APP_IN_ARCHIVE/Contents/Frameworks/Sparkle.framework"

    # Remove quarantine attributes from all files (ITMS-91109)
    echo "Removing quarantine attributes..."
    xattr -cr "$APP_IN_ARCHIVE"
fi

echo "Archive created: $ARCHIVE_PATH"

# Exit here if test mode
if [ "$MODE" == "--test" ]; then
    echo ""
    echo "=== Test archive complete ==="
    echo "Archive location: $ARCHIVE_PATH"
    echo ""
    echo "To manually upload, run:"
    echo "  xcodebuild -exportArchive -archivePath \"$ARCHIVE_PATH\" -exportOptionsPlist ExportOptions-AppStore.plist -exportPath \"$EXPORT_PATH\""
    exit 0
fi

# Step 3: Export for App Store
echo ""
echo "[3/4] Exporting for App Store..."
xcodebuild -exportArchive \
    -archivePath "$ARCHIVE_PATH" \
    -exportOptionsPlist ExportOptions-AppStore.plist \
    -exportPath "$EXPORT_PATH"

# Step 4: Verify upload
echo ""
echo "[4/4] Upload complete!"
echo "The app was uploaded to App Store Connect during the export step."

echo ""
echo "=== App Store Release v${VERSION} (Build ${BUILD}) Complete ==="
echo ""
echo "Next steps:"
echo "1. Go to App Store Connect: https://appstoreconnect.apple.com"
echo "2. Select Nottawa > TestFlight or App Store tab"
echo "3. The build should appear within a few minutes"
echo "4. Complete compliance questionnaire if prompted"
echo "5. Submit for review"
