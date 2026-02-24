#!/bin/bash

# Nottawa Release Script
# Handles the full release process: build, sign, upload, and deploy
#
# Usage:
#   ./release.sh VERSION           - Full release (build + upload + deploy)
#   ./release.sh VERSION --upload  - Upload existing DMG only (skip build)
#   ./release.sh VERSION --test    - Build only, no upload
#
# Example:
#   ./release.sh 1.0.2

set -e

VERSION=$1
MODE=$2
SERVER="root@165.227.44.1"
SERVER_DIR="~/nottawa-server"
LOCAL_SERVER_DIR="$HOME/dev/nottawa-server"
DMG_NAME="Nottawa-${VERSION}.dmg"

if [ -z "$VERSION" ]; then
    echo "Usage: ./release.sh VERSION [--upload|--test]"
    echo ""
    echo "Examples:"
    echo "  ./release.sh 1.0.2           # Full release"
    echo "  ./release.sh 1.0.2 --upload  # Upload existing DMG only"
    echo "  ./release.sh 1.0.2 --test    # Build only, no upload"
    exit 1
fi

echo "=== Nottawa Release v${VERSION} ==="

# Step 1: Build (unless --upload mode)
if [ "$MODE" != "--upload" ]; then
    echo ""
    echo "[1/4] Building release..."

    # Clean previous builds
    rm -rf build/Build

    # Build the app
    xcodebuild \
        -project "dude_wake_up.xcodeproj" \
        -scheme "dude_wake_up Release" \
        -configuration Release \
        -derivedDataPath build \
        clean build

    APP_PATH="bin/Nottawa.app"
    if [ ! -d "$APP_PATH" ]; then
        echo "Error: Build failed. App not found at $APP_PATH"
        exit 1
    fi

    # Update version
    /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $VERSION" "$APP_PATH/Contents/Info.plist"
    /usr/libexec/PlistBuddy -c "Set :CFBundleVersion $VERSION" "$APP_PATH/Contents/Info.plist"

    # Code sign
    echo "Code signing..."
    codesign --deep --force --verify --verbose \
        --sign "Developer ID Application" \
        --options runtime \
        --entitlements dude_wake_up.entitlements \
        "$APP_PATH"

    # Create DMG
    echo "Creating DMG..."
    rm -f "$DMG_NAME"
    TEMP_DIR=$(mktemp -d)
    cp -R "$APP_PATH" "$TEMP_DIR/"
    hdiutil create -volname "Nottawa $VERSION" -srcfolder "$TEMP_DIR" -ov -format UDZO "$DMG_NAME"
    rm -rf "$TEMP_DIR"

    # Notarize (if not test mode)
    if [ "$MODE" != "--test" ]; then
        echo "Notarizing..."
        xcrun notarytool submit "$DMG_NAME" \
            --keychain-profile "AC_PASSWORD" \
            --wait
        xcrun stapler staple "$DMG_NAME"
    fi

    echo "Build complete: $DMG_NAME ($(du -h "$DMG_NAME" | cut -f1))"
fi

# Exit here if test mode
if [ "$MODE" == "--test" ]; then
    echo ""
    echo "=== Test build complete ==="
    exit 0
fi

# Step 2: Sign for Sparkle and get signature
echo ""
echo "[2/4] Signing for Sparkle..."
if [ ! -f "$DMG_NAME" ]; then
    echo "Error: $DMG_NAME not found"
    exit 1
fi

SIGNATURE=$(./lib/Sparkle.framework/bin/sign_update "$DMG_NAME" --account ed25519 -p 2>/dev/null)
if [ -z "$SIGNATURE" ]; then
    echo "Error: Failed to sign update"
    exit 1
fi

DMG_SIZE=$(stat -f%z "$DMG_NAME")
PUB_DATE=$(date -R)

# Step 3: Upload DMG to server
echo ""
echo "[3/4] Uploading DMG to server..."
scp "$DMG_NAME" "${SERVER}:${SERVER_DIR}/downloads/"

# Step 4: Update appcast.xml and deploy
echo ""
echo "[4/4] Updating appcast and deploying..."

# Update local appcast
APPCAST="$LOCAL_SERVER_DIR/nottawa-website/src/appcast.njk"
NEW_ITEM="
    <!-- Version $VERSION -->
    <item>
      <title>Version $VERSION</title>
      <sparkle:releaseNotesLink>
        https://nottawa.app/release-notes/${VERSION}.html
      </sparkle:releaseNotesLink>
      <pubDate>$PUB_DATE</pubDate>
      <enclosure
        url=\"https://nottawa.app/downloads/$DMG_NAME\"
        sparkle:version=\"$VERSION\"
        sparkle:shortVersionString=\"$VERSION\"
        sparkle:edSignature=\"$SIGNATURE\"
        length=\"$DMG_SIZE\"
        type=\"application/octet-stream\" />
      <sparkle:minimumSystemVersion>10.13</sparkle:minimumSystemVersion>
    </item>"

# Insert after "<!-- Latest version first -->"
awk -v item="$NEW_ITEM" '
    /<!-- Latest version first -->/ {
        print
        print item
        next
    }
    { print }
' "$APPCAST" > "${APPCAST}.tmp" && mv "${APPCAST}.tmp" "$APPCAST"

# Update download page version
DOWNLOAD_PAGE="$LOCAL_SERVER_DIR/nottawa-website/src/download.html"
sed -i '' "s|/downloads/Nottawa-[0-9.]*\.dmg|/downloads/$DMG_NAME|g" "$DOWNLOAD_PAGE"
sed -i '' "s|Download \.dmg (v[0-9.]*)|Download .dmg (v$VERSION)|g" "$DOWNLOAD_PAGE"

# Create release notes if needed
RELEASE_NOTES="$LOCAL_SERVER_DIR/nottawa-website/src/release-notes/${VERSION}.html"
if [ ! -f "$RELEASE_NOTES" ]; then
    cat > "$RELEASE_NOTES" << EOF
---
permalink: /release-notes/${VERSION}.html
eleventyExcludeFromCollections: true
---
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Nottawa ${VERSION} Release Notes</title>
    <style>
        body { font-family: -apple-system, sans-serif; line-height: 1.6; color: #e0e0e0; background: #1a1a1a; max-width: 600px; margin: 0 auto; padding: 20px; }
        h1 { color: #fff; border-bottom: 2px solid #9333ea; padding-bottom: 10px; }
        .new { color: #34d399; font-weight: bold; }
        .fixed { color: #f87171; font-weight: bold; }
    </style>
</head>
<body>
    <h1>Nottawa ${VERSION}</h1>
    <p><strong>Release Date:</strong> $(date '+%B %d, %Y')</p>
    <h2>What's New</h2>
    <ul>
        <li><span class="new">NEW:</span> TODO: Add release notes</li>
    </ul>
</body>
</html>
EOF
    echo "Created release notes template: $RELEASE_NOTES"
    echo "Please edit before final deploy!"
fi

# Deploy server
echo "Deploying server..."
cd "$LOCAL_SERVER_DIR"
./deploy.sh

echo ""
echo "=== Release v${VERSION} Complete ==="
echo ""
echo "Download URL: https://nottawa.app/downloads/$DMG_NAME"
echo "Release Notes: https://nottawa.app/release-notes/${VERSION}.html"
echo ""
echo "Don't forget to edit the release notes if you haven't already!"
