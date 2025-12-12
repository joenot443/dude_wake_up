#!/bin/bash

# Nottawa Update Deployment Script
# This script deploys a new version to the nottawa-server

set -e

VERSION=$1
DMG_FILE="Nottawa-${VERSION}.dmg"
SERVER_DIR="$HOME/dev/nottawa-server"

if [ -z "$VERSION" ]; then
  echo "Usage: ./deploy_update.sh VERSION"
  echo "Example: ./deploy_update.sh 1.0.2"
  exit 1
fi

# Check if DMG exists
if [ ! -f "$DMG_FILE" ]; then
  echo "Error: $DMG_FILE not found. Please build it first with ./build_release.sh"
  exit 1
fi

echo "Deploying version $VERSION to nottawa-server..."

# 1. Sign the DMG for Sparkle
echo "Signing DMG for Sparkle..."
# Use the keychain-stored key (account name: ed25519)
SIGNATURE=$(./lib/Sparkle.framework/bin/sign_update "$DMG_FILE" --account ed25519 -p 2>/dev/null || echo "")

if [ -z "$SIGNATURE" ]; then
  echo "Error: Failed to sign update"
  exit 1
fi

# 2. Copy DMG to server
echo "Copying DMG to server..."
cp "$DMG_FILE" "$SERVER_DIR/nottawa-website/src/downloads/"

# 3. Create release notes if they don't exist
RELEASE_NOTES="$SERVER_DIR/nottawa-website/src/release-notes/${VERSION}.html"
if [ ! -f "$RELEASE_NOTES" ]; then
  echo "Creating release notes template..."
  cat > "$RELEASE_NOTES" << 'EOF'
---
permalink: /release-notes/VERSION.html
eleventyExcludeFromCollections: true
---
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Nottawa VERSION Release Notes</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            line-height: 1.6;
            color: #e0e0e0;
            background: #1a1a1a;
            max-width: 600px;
            margin: 0 auto;
            padding: 20px;
        }
        h1 {
            color: #ffffff;
            border-bottom: 2px solid #9333ea;
            padding-bottom: 10px;
        }
        h2 {
            color: #e0e0e0;
            margin-top: 25px;
        }
        ul {
            padding-left: 20px;
        }
        li {
            margin: 8px 0;
        }
        .new {
            color: #34d399;
            font-weight: bold;
        }
        .improved {
            color: #60a5fa;
            font-weight: bold;
        }
        .fixed {
            color: #f87171;
            font-weight: bold;
        }
        .note {
            background: linear-gradient(135deg, #9333ea 0%, #6366f1 100%);
            color: white;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <h1>Nottawa VERSION</h1>
    <p><strong>Release Date:</strong> DATE</p>

    <h2>What's New</h2>
    <ul>
        <li><span class="new">NEW:</span> Add new features here</li>
    </ul>

    <h2>Improvements</h2>
    <ul>
        <li><span class="improved">IMPROVED:</span> Add improvements here</li>
    </ul>

    <h2>Bug Fixes</h2>
    <ul>
        <li><span class="fixed">FIXED:</span> Add bug fixes here</li>
    </ul>

    <div class="note">
        <strong>Note:</strong> This version requires macOS 10.13 or later.
    </div>
</body>
</html>
EOF

  # Replace VERSION and DATE placeholders
  sed -i '' "s/VERSION/${VERSION}/g" "$RELEASE_NOTES"
  sed -i '' "s/DATE/$(date '+%B %d, %Y')/g" "$RELEASE_NOTES"

  echo "Please edit $RELEASE_NOTES with actual release notes!"
fi

# 4. Update appcast.xml
echo "Updating appcast.xml..."
APPCAST="$SERVER_DIR/nottawa-website/src/appcast.njk"
DMG_SIZE=$(stat -f%z "$DMG_FILE")
PUB_DATE=$(date -R)

# Create new item entry
NEW_ITEM=$(cat << EOF

    <!-- Version $VERSION -->
    <item>
      <title>Version $VERSION</title>
      <sparkle:releaseNotesLink>
        https://nottawa.app/release-notes/${VERSION}.html
      </sparkle:releaseNotesLink>
      <pubDate>$PUB_DATE</pubDate>
      <enclosure
        url="https://nottawa.app/downloads/$DMG_FILE"
        sparkle:version="$VERSION"
        sparkle:shortVersionString="$VERSION"
        sparkle:edSignature="$SIGNATURE"
        length="$DMG_SIZE"
        type="application/octet-stream" />
      <sparkle:minimumSystemVersion>10.13</sparkle:minimumSystemVersion>
    </item>
EOF
)

# Insert new item after <!-- Latest version first -->
awk '
  /<!-- Latest version first -->/ {
    print
    print "'"$NEW_ITEM"'"
    next
  }
  { print }
' "$APPCAST" > "${APPCAST}.tmp" && mv "${APPCAST}.tmp" "$APPCAST"

# 5. Update download page
echo "Updating download page..."
DOWNLOAD_PAGE="$SERVER_DIR/nottawa-website/src/download.html"
sed -i '' "s|/downloads/Nottawa-[0-9.]*\.dmg|/downloads/$DMG_FILE|g" "$DOWNLOAD_PAGE"
sed -i '' "s|Download \.dmg (v[0-9.]*)|Download .dmg (v$VERSION)|g" "$DOWNLOAD_PAGE"

# 6. Build and deploy
echo "Building website..."
cd "$SERVER_DIR/nottawa-website"
npx eleventy

# Copy downloads to _site
cp -r src/downloads _site/

echo ""
echo "========================================="
echo "Update Deployed Successfully!"
echo "========================================="
echo "Version: $VERSION"
echo "DMG: $DMG_FILE"
echo "Signature: $SIGNATURE"
echo ""
echo "Next steps:"
echo "1. Edit release notes if needed: $RELEASE_NOTES"
echo "2. Commit and push changes to git"
echo "3. Deploy to production server"
echo ""
echo "To test locally:"
echo "  cd $SERVER_DIR && npm start"
echo "  Then check: http://localhost:3000/appcast.xml"