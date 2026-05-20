#!/bin/bash
#
# Build, sign, notarize, and DMG-package a direct-distribution copy of
# Nottawa (the Developer ID build that ships from nottawa.app — distinct
# from the Mac App Store build).
#
# Usage:
#   scripts/release.sh 2.0.0
#
# Credentials (need at least one method):
#   1. Keychain profile (preferred, set up once):
#        xcrun notarytool store-credentials nottawa-notary \
#          --apple-id you@example.com --team-id 7VUCELU5PL
#      Then: export NOTARY_PROFILE=nottawa-notary
#
#   2. Inline env vars (one-shot):
#        export APPLE_ID=you@example.com
#        export APPLE_TEAM_ID=7VUCELU5PL
#        export APP_SPECIFIC_PASSWORD=xxxx-xxxx-xxxx-xxxx
#
# Output:
#   dist/Nottawa-<VERSION>.dmg          — what to upload to nottawa.app
#   dist/Nottawa-<VERSION>.xcarchive    — keep for crash-report symbolication
#   dist/Nottawa.app                    — stapled .app (the DMG's contents)
#
# Pitfalls this script encodes (do not undo):
#   - Use `ditto`, never `cp -R`, to populate the DMG staging folder.
#     `cp -R` strips macOS-native metadata and produces DMGs that Finder
#     refuses to drag with "items had to be skipped" errors.
#   - Build the DMG with `-fs APFS`. HFS+ from a project on APFS triggers
#     the same skipped-items error during user drag-to-Applications.
#   - Strip .DS_Store files from inside the .app before staging — they
#     enter via copy_resources.sh's `cp -R "$NOTTAWA_DIR/shaders/" ...`
#     glob and don't belong in shipped bundles.
#   - Notarize BOTH the .app and the .dmg. Apple recommends only the .dmg
#     now, but stapling the .app too means it verifies on offline Macs
#     where Gatekeeper can't fetch the ticket online.
#   - A 403 from notarytool ("required agreement is missing or has
#     expired") is account-level, not build-level. Sign in to
#     developer.apple.com and appstoreconnect.apple.com → Agreements,
#     Tax, and Banking, accept anything pending, retry.

set -euo pipefail

VERSION="${1:-}"
if [[ -z "$VERSION" ]]; then
  echo "usage: $0 VERSION   (e.g. $0 2.0.0)" >&2
  exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

DIST="$PROJECT_ROOT/dist"
ARCHIVE="$DIST/Nottawa-${VERSION}.xcarchive"
APP="$DIST/Nottawa.app"
DMG="$DIST/Nottawa-${VERSION}.dmg"
ZIP="$DIST/Nottawa.zip"           # transient, for .app notarization upload
STAGING="$DIST/staging"
EXPORT_PLIST="$DIST/ExportOptions.plist"
SIGN_IDENTITY="Developer ID Application: Joe Crozier (7VUCELU5PL)"
TEAM_ID="7VUCELU5PL"

# Decide how to authenticate with notarytool.
notarize() {
  local input="$1"
  if [[ -n "${NOTARY_PROFILE:-}" ]]; then
    xcrun notarytool submit "$input" --keychain-profile "$NOTARY_PROFILE" --wait
  elif [[ -n "${APPLE_ID:-}" && -n "${APP_SPECIFIC_PASSWORD:-}" ]]; then
    xcrun notarytool submit "$input" \
      --apple-id "$APPLE_ID" \
      --team-id "${APPLE_TEAM_ID:-$TEAM_ID}" \
      --password "$APP_SPECIFIC_PASSWORD" \
      --wait
  else
    echo "error: no notarization credentials." >&2
    echo "       set NOTARY_PROFILE, or APPLE_ID + APP_SPECIFIC_PASSWORD." >&2
    exit 2
  fi
}

echo "==> Cleaning previous artifacts"
rm -rf "$ARCHIVE" "$APP" "$DMG" "$ZIP" "$STAGING"
mkdir -p "$DIST"

if [[ ! -f "$EXPORT_PLIST" ]]; then
  cat > "$EXPORT_PLIST" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>developer-id</string>
    <key>teamID</key>
    <string>7VUCELU5PL</string>
    <key>signingStyle</key>
    <string>automatic</string>
    <key>destination</key>
    <string>export</string>
</dict>
</plist>
PLIST
fi

echo "==> Archiving Release configuration"
xcodebuild \
  -project dude_wake_up.xcodeproj \
  -scheme NottawaApp \
  -configuration Release \
  -archivePath "$ARCHIVE" \
  archive | tail -1

echo "==> Exporting Developer ID signed .app"
xcodebuild \
  -exportArchive \
  -archivePath "$ARCHIVE" \
  -exportPath "$DIST/export" \
  -exportOptionsPlist "$EXPORT_PLIST" | tail -1
mv "$DIST/export/NottawaApp.app" "$APP"
rm -rf "$DIST/export"

echo "==> Stripping .DS_Store noise from bundle"
find "$APP" -name ".DS_Store" -delete

# Belt-and-suspenders quarantine scrub. NottawaApp's copy_resources.sh build
# phase already runs `xattr -cr` on the bundle pre-codesign, but if a future
# Xcode build setting bypasses that script (e.g. an alternate target), this
# catches it. Quarantine xattrs on any inner file trigger ITMS-91109 when
# uploading to the App Store and aren't desirable for direct distribution
# either. xattr changes don't invalidate codesignatures.
echo "==> Stripping extended attributes (quarantine guard)"
xattr -cr "$APP"

echo "==> Verifying signature"
codesign --verify --deep --strict --verbose=2 "$APP" 2>&1 | tail -2

echo "==> Notarizing .app"
ditto -c -k --keepParent --sequesterRsrc "$APP" "$ZIP"
notarize "$ZIP" | tail -5
rm -f "$ZIP"
xcrun stapler staple "$APP" | tail -1

echo "==> Building DMG (APFS, ditto-staged)"
mkdir -p "$STAGING"
ditto "$APP" "$STAGING/Nottawa.app"
ln -s /Applications "$STAGING/Applications"
hdiutil create \
  -volname "Nottawa" \
  -srcfolder "$STAGING" \
  -fs APFS \
  -format UDZO \
  -ov \
  "$DMG" | tail -1

echo "==> Signing DMG"
codesign --sign "$SIGN_IDENTITY" --options runtime --timestamp "$DMG"

echo "==> Notarizing DMG"
notarize "$DMG" | tail -5
xcrun stapler staple "$DMG" | tail -1

echo "==> Final verification"
spctl --assess --type install --verbose "$DMG"
spctl --assess --type execute --verbose "$APP"

echo ""
echo "==> Done. Ready to upload:"
ls -lh "$DMG"
