#!/bin/bash

# Nottawa Release Build Script with Sparkle Integration
# Usage: ./build_release.sh VERSION [--test]

set -e  # Exit on error

VERSION=$1
TEST_MODE=$2

if [ -z "$VERSION" ]; then
  echo "Usage: ./build_release.sh VERSION [--test]"
  echo "Example: ./build_release.sh 1.0.3"
  echo "         ./build_release.sh 1.0.3 --test"
  exit 1
fi

echo "Building Nottawa version $VERSION..."

# Configuration
PROJECT_NAME="dude_wake_up"
APP_NAME="Nottawa"
SCHEME="${PROJECT_NAME}"
DMG_NAME="${APP_NAME}-${VERSION}.dmg"
# Note: Sparkle private key should be stored in the keychain under account "ed25519"

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build/Build

# Build the app
echo "Building Release configuration..."
xcodebuild \
  -project "${PROJECT_NAME}.xcodeproj" \
  -scheme "$SCHEME" \
  -configuration Release \
  -derivedDataPath build \
  PRODUCT_BUNDLE_IDENTIFIER="com.nottawa.app" \
  DEVELOPMENT_TEAM="YOUR_TEAM_ID" \
  clean build

# Check if build succeeded
APP_PATH="build/Build/Products/Release/${APP_NAME}.app"
if [ ! -d "$APP_PATH" ]; then
  echo "Error: Build failed. App not found at $APP_PATH"
  exit 1
fi

# Update version in Info.plist
echo "Updating version to $VERSION..."
/usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $VERSION" "$APP_PATH/Contents/Info.plist"
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $VERSION" "$APP_PATH/Contents/Info.plist"

# Code sign the app
echo "Code signing..."
codesign --deep --force --verify --verbose \
  --sign "Developer ID Application: YOUR_NAME (YOUR_TEAM_ID)" \
  --options runtime \
  --entitlements dude_wake_up.entitlements \
  "$APP_PATH"

# Verify code signing
echo "Verifying code signature..."
codesign --verify --deep --strict "$APP_PATH"

# Create DMG
echo "Creating DMG..."
# Remove old DMG if it exists
rm -f "$DMG_NAME"

# Create a temporary directory for the DMG
TEMP_DIR=$(mktemp -d)
cp -R "$APP_PATH" "$TEMP_DIR/"

# Create DMG using hdiutil
hdiutil create \
  -volname "$APP_NAME $VERSION" \
  -srcfolder "$TEMP_DIR" \
  -ov \
  -format UDZO \
  "$DMG_NAME"

# Clean up temp directory
rm -rf "$TEMP_DIR"

# Notarize the DMG (requires Apple Developer account)
if [ "$TEST_MODE" != "--test" ]; then
  echo "Notarizing DMG..."
  xcrun notarytool submit "$DMG_NAME" \
    --apple-id "your-apple-id@example.com" \
    --team-id "YOUR_TEAM_ID" \
    --password "@keychain:AC_PASSWORD" \
    --wait

  # Staple the notarization ticket
  echo "Stapling notarization..."
  xcrun stapler staple "$DMG_NAME"
fi

# Sign for Sparkle
if [ "$TEST_MODE" != "--test" ]; then
  echo "Signing update for Sparkle..."
  # Use the keychain-stored key (account name: ed25519)
  SIGNATURE=$(./lib/Sparkle.framework/bin/sign_update "$DMG_NAME" --account ed25519 -p)

  echo ""
  echo "========================================="
  echo "Build Complete!"
  echo "========================================="
  echo "Version: $VERSION"
  echo "DMG: $DMG_NAME"
  echo "Size: $(du -h "$DMG_NAME" | cut -f1)"
  echo ""
  echo "Add this to your appcast.xml:"
  echo ""
  cat << EOF
<item>
  <title>Version $VERSION</title>
  <sparkle:releaseNotesLink>
    https://your-server.com/nottawa/release-notes/$VERSION.html
  </sparkle:releaseNotesLink>
  <pubDate>$(date -R)</pubDate>
  <enclosure
    url="https://your-server.com/nottawa/$DMG_NAME"
    sparkle:version="$VERSION"
    sparkle:shortVersionString="$VERSION"
    sparkle:edSignature="$SIGNATURE"
    length="$(stat -f%z "$DMG_NAME")"
    type="application/octet-stream" />
  <sparkle:minimumSystemVersion>10.13</sparkle:minimumSystemVersion>
</item>
EOF
else
  echo ""
  echo "========================================="
  echo "Test Build Complete!"
  echo "========================================="
  echo "Version: $VERSION"
  echo "DMG: $DMG_NAME"
  echo "Size: $(du -h "$DMG_NAME" | cut -f1)"
  echo ""
  echo "Note: This is a test build. To create a signed release:"
  echo "1. Save your Sparkle private key to $PRIVATE_KEY_FILE"
  echo "2. Run without --test flag"
fi