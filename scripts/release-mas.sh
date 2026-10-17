#!/bin/bash
#
# Build, sign, and upload a Mac App Store (App Store Connect) build of
# Nottawa. This is the MAS submission path — DISTINCT from the Developer
# ID / nottawa.app DMG path in scripts/release.sh.
#
# What it does:
#   1. Bumps CURRENT_PROJECT_VERSION (build number) on the NottawaApp target.
#   2. Archives the Release configuration.
#   3. Exports a .pkg with method=app-store-connect (cloud-managed Apple
#      Distribution signing, fetched automatically).
#   4. Validates the .pkg against App Store Connect.
#   5. Uploads it (unless --validate-only).
#
# Usage:
#   scripts/release-mas.sh <version> [build]
#     <version>  marketing version that MUST already be set on the NottawaApp
#                target (e.g. 2.0.0). The script asserts the archive matches;
#                it does NOT change MARKETING_VERSION — bump that by hand in
#                project.pbxproj when you ship a new user-facing version (rare).
#     [build]    optional build number. Default = current NottawaApp build + 1.
#
#   Flags (env):
#     VALIDATE_ONLY=1     stop after --validate-app; don't upload.
#     SKIP_BUMP=1         don't touch the build number (re-run a failed upload
#                         of the same build).
#
# Credentials (App Store Connect upload):
#   Apple ID defaults to joe.crozier@live.ca (override with APPLE_ID).
#   Password resolves in this order:
#     1. APP_SPECIFIC_PASSWORD env var (literal app-specific password), else
#     2. @keychain:AC_PASSWORD  (store once with:
#          security add-generic-password -s AC_PASSWORD \
#            -a joe.crozier@live.ca -w xxxx-xxxx-xxxx-xxxx )
#   altool CANNOT use the `nottawa-notary` notarytool keychain profile — that
#   profile format is notarytool-only.
#
# Output:
#   dist/Nottawa-<version>-b<build>.xcarchive   keep for crash symbolication
#   dist/mas-export-b<build>/NottawaApp.pkg     the uploaded package
#
# Notes / gotchas this encodes:
#   - ASC rejects a re-used build number. Always bump (this script does).
#   - The build number bump edits project.pbxproj but does NOT commit. Commit
#     it yourself so the repo records what shipped:
#       git add dude_wake_up.xcodeproj/project.pbxproj
#       git commit -m "Bump NottawaApp build to <n> for App Store submission"
#   - No local "Apple Distribution" identity is needed; automatic signing
#     fetches the Cloud Managed Apple Distribution cert at export time. The
#     personal-team "Apple Development" cert shown by `security find-identity`
#     is a red herring.
#   - A 403 from Apple is account-level (a Program License / Paid Apps
#     agreement needs accepting at developer.apple.com AND
#     appstoreconnect.apple.com -> Agreements, Tax, and Banking), not a build
#     problem. Accept pending agreements in BOTH and retry.

set -euo pipefail

VERSION="${1:-}"
NEW_BUILD="${2:-}"
if [[ -z "$VERSION" ]]; then
  echo "usage: $0 <version> [build]   (e.g. $0 2.0.0   or   $0 2.0.0 35)" >&2
  exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

PBX="$PROJECT_ROOT/dude_wake_up.xcodeproj/project.pbxproj"
DIST="$PROJECT_ROOT/dist"
MAS_PLIST="$DIST/ExportOptions-MAS.plist"
APPLE_ID="${APPLE_ID:-joe.crozier@live.ca}"
TEAM_ID="7VUCELU5PL"
mkdir -p "$DIST"

# Resolve the App Store Connect password reference for altool.
if [[ -n "${APP_SPECIFIC_PASSWORD:-}" ]]; then
  ALTOOL_PW="$APP_SPECIFIC_PASSWORD"
else
  ALTOOL_PW="@keychain:AC_PASSWORD"
fi

# --- Determine current build number ---------------------------------------
# The NottawaApp target carries the unique HIGHEST CURRENT_PROJECT_VERSION and
# appears exactly twice (Debug + Release). Other targets use 1/2/30. We assert
# the count is 2 so a project change can never make this silently edit the
# wrong target.
CUR_BUILD="$(grep -oE 'CURRENT_PROJECT_VERSION = [0-9]+;' "$PBX" \
  | grep -oE '[0-9]+' | sort -n | tail -1)"
CUR_COUNT="$(grep -cE "CURRENT_PROJECT_VERSION = ${CUR_BUILD};" "$PBX")"
if [[ "$CUR_COUNT" != "2" ]]; then
  echo "error: highest CURRENT_PROJECT_VERSION ($CUR_BUILD) appears $CUR_COUNT" \
       "times, expected 2 (NottawaApp Debug+Release)." >&2
  echo "       Refusing to auto-bump. Bump the build number by hand." >&2
  exit 2
fi

if [[ "${SKIP_BUMP:-}" == "1" ]]; then
  NEW_BUILD="$CUR_BUILD"
  echo "==> SKIP_BUMP set; keeping build $NEW_BUILD"
else
  NEW_BUILD="${NEW_BUILD:-$((CUR_BUILD + 1))}"
  if [[ "$NEW_BUILD" -le "$CUR_BUILD" ]]; then
    echo "error: requested build $NEW_BUILD is not greater than current" \
         "$CUR_BUILD; ASC will reject a re-used build number." >&2
    exit 2
  fi
  echo "==> Bumping NottawaApp build $CUR_BUILD -> $NEW_BUILD"
  sed -i '' "s/CURRENT_PROJECT_VERSION = ${CUR_BUILD};/CURRENT_PROJECT_VERSION = ${NEW_BUILD};/g" "$PBX"
fi

ARCHIVE="$DIST/Nottawa-${VERSION}-b${NEW_BUILD}.xcarchive"
EXPORT_DIR="$DIST/mas-export-b${NEW_BUILD}"
PKG="$EXPORT_DIR/NottawaApp.pkg"

# --- Ensure the MAS export options plist exists ----------------------------
if [[ ! -f "$MAS_PLIST" ]]; then
  cat > "$MAS_PLIST" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>app-store-connect</string>
    <key>teamID</key>
    <string>${TEAM_ID}</string>
    <key>signingStyle</key>
    <string>automatic</string>
    <key>destination</key>
    <string>export</string>
    <key>uploadSymbols</key>
    <true/>
</dict>
</plist>
PLIST
fi

# --- Archive ----------------------------------------------------------------
echo "==> Archiving Release configuration (this takes a few minutes)"
rm -rf "$ARCHIVE" "$EXPORT_DIR"
xcodebuild \
  -project dude_wake_up.xcodeproj \
  -scheme NottawaApp \
  -configuration Release \
  -archivePath "$ARCHIVE" \
  archive | tail -1

# --- Verify the archive matches what we intend to ship ---------------------
A_VER="$(/usr/libexec/PlistBuddy -c "Print :ApplicationProperties:CFBundleShortVersionString" "$ARCHIVE/Info.plist")"
A_BUILD="$(/usr/libexec/PlistBuddy -c "Print :ApplicationProperties:CFBundleVersion" "$ARCHIVE/Info.plist")"
echo "==> Archived version $A_VER build $A_BUILD"
if [[ "$A_BUILD" != "$NEW_BUILD" ]]; then
  echo "error: archived build ($A_BUILD) != expected ($NEW_BUILD)." >&2
  exit 3
fi
if [[ "$A_VER" != "$VERSION" ]]; then
  echo "error: archived marketing version ($A_VER) != requested ($VERSION)." >&2
  echo "       This script does not change MARKETING_VERSION. Set it on the" >&2
  echo "       NottawaApp target in project.pbxproj, then re-run." >&2
  exit 3
fi

# --- Export the App Store .pkg ---------------------------------------------
echo "==> Exporting app-store-connect package"
xcodebuild \
  -exportArchive \
  -archivePath "$ARCHIVE" \
  -exportPath "$EXPORT_DIR" \
  -exportOptionsPlist "$MAS_PLIST" | tail -1

if [[ ! -f "$PKG" ]]; then
  echo "error: export did not produce $PKG" >&2
  exit 4
fi

echo "==> Package signature"
pkgutil --check-signature "$PKG" | sed -n '1,3p'

# --- Validate ---------------------------------------------------------------
echo "==> Validating with App Store Connect"
xcrun altool --validate-app -f "$PKG" -t macos \
  -u "$APPLE_ID" -p "$ALTOOL_PW" 2>&1 | tail -8

if [[ "${VALIDATE_ONLY:-}" == "1" ]]; then
  echo ""
  echo "==> VALIDATE_ONLY set; not uploading. Package ready at:"
  ls -lh "$PKG"
  exit 0
fi

# --- Upload -----------------------------------------------------------------
echo "==> Uploading to App Store Connect"
xcrun altool --upload-app -f "$PKG" -t macos \
  -u "$APPLE_ID" -p "$ALTOOL_PW" 2>&1 | tail -10

echo ""
echo "==> Done. Build $NEW_BUILD (v$VERSION) uploaded."
echo "    It appears in App Store Connect -> Builds after processing."
if [[ "${SKIP_BUMP:-}" != "1" ]]; then
  echo "    Remember to commit the build bump:"
  echo "      git add dude_wake_up.xcodeproj/project.pbxproj"
  echo "      git commit -m \"Bump NottawaApp build to $NEW_BUILD for App Store submission (v$VERSION)\""
fi
