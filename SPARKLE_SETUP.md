# Sparkle Auto-Update Setup for Nottawa

## Overview

Nottawa is now configured to use Sparkle for automatic updates. The update server is integrated with your existing nottawa-server at nottawa.app.

## Architecture

```
Nottawa App → checks → https://nottawa.app/appcast.xml
                           ↓
                    Lists available updates
                           ↓
            Downloads DMG from https://nottawa.app/downloads/
```

## Files Created

### In Nottawa Project (`dude_wake_up`)
- `src/Services/SparkleUpdater.h` - C++/Objective-C++ wrapper header
- `src/Services/SparkleUpdater.mm` - Implementation with dual interfaces
- `openFrameworks-Info.plist` - Updated with Sparkle configuration
- `build_release.sh` - Build and sign releases
- `deploy_update.sh` - Deploy updates to server

### In Server (`nottawa-server`)
- `nottawa-website/src/appcast.njk` - Update feed (builds to appcast.xml)
- `nottawa-website/src/downloads/` - Directory for DMG files
- `nottawa-website/src/release-notes/` - Release notes HTML files
- `nottawa-website/src/download.html` - Updated with direct download link

## Setup Instructions

### 1. Add Sparkle Framework to Xcode

```bash
# Download Sparkle
curl -L -O https://github.com/sparkle-project/Sparkle/releases/download/2.6.4/Sparkle-2.6.4.tar.xz
tar -xf Sparkle-2.6.4.tar.xz
cp -R Sparkle.framework lib/
rm -rf Sparkle-2.6.4.tar.xz
```

In Xcode:
1. Drag `lib/Sparkle.framework` into the project
2. Set to "Embed & Sign" in Build Phases
3. Add `src/Services/SparkleUpdater.h` and `.mm` files to the project

### 2. Generate Keys

```bash
# Generate EdDSA keys
./lib/Sparkle.framework/bin/generate_keys

# The key is automatically saved to macOS Keychain (account: ed25519)
# Add the public key to Info.plist (already done: 2ISmClcG8GYubpCKqQpOqRMOYqHmQKOjo3XOjW9z7Nk=)
```

### 3. Update Application Code

In `src/Apps/MainApp.cpp`, add after `StringManager::loadStrings()`:
```cpp
#include "Services/SparkleUpdater.h"

// In setup()
#ifdef TARGET_OSX
SparkleUpdaterService::getInstance().initialize();
#endif
```

In `src/Views/MainStageView.cpp`, add menu item after "Send Feedback":
```cpp
#include "../Services/SparkleUpdater.h"

// In menu code
#ifdef TARGET_OSX
if (ImGui::MenuItem("Check for Updates..."))
{
    SparkleUpdaterService::getInstance().checkForUpdates();
}
#endif
```

### 4. Configure Build Script

Edit `build_release.sh` and update:
- `DEVELOPMENT_TEAM="YOUR_TEAM_ID"`
- `--sign "Developer ID Application: YOUR_NAME (YOUR_TEAM_ID)"`
- `--apple-id "your-apple-id@example.com"`

## Releasing Updates

### 1. Build a Release

```bash
# Build and sign a new version
./build_release.sh 1.0.2

# This will:
# - Build the app in Release mode
# - Update version numbers
# - Code sign the app
# - Create a DMG
# - Sign for Sparkle
# - Output appcast XML snippet
```

### 2. Deploy to Server

```bash
# Deploy the update to nottawa-server
./deploy_update.sh 1.0.2

# This will:
# - Copy DMG to server
# - Create/update release notes
# - Update appcast.xml
# - Update download page
# - Build the website
```

### 3. Edit Release Notes

Edit the generated release notes file:
```
~/dev/nottawa-server/nottawa-website/src/release-notes/1.0.2.html
```

### 4. Deploy to Production

```bash
cd ~/dev/nottawa-server
git add .
git commit -m "Release version 1.0.2"
git push

# Then deploy to production server
```

## Server URLs

- **Appcast Feed**: https://nottawa.app/appcast.xml
- **Downloads**: https://nottawa.app/downloads/Nottawa-VERSION.dmg
- **Release Notes**: https://nottawa.app/release-notes/VERSION.html

## Testing

### Local Testing

1. Start local server:
```bash
cd ~/dev/nottawa-server
npm start
```

2. Temporarily update Info.plist:
```xml
<key>SUFeedURL</key>
<string>http://localhost:3000/appcast.xml</string>
```

3. Build app with lower version number to test updates

### Production Testing

1. Install current production version
2. Run "Check for Updates..." from menu
3. Verify update notification appears
4. Test download and installation

## Security Notes

- **Private Key**: Keep `~/.nottawa/sparkle_private_key` secure
- **HTTPS Only**: Always serve updates over HTTPS in production
- **Code Signing**: Ensure app is properly signed with Developer ID
- **Notarization**: Required for macOS 10.15+

## Troubleshooting

### Update Check Fails
- Check Console.app for Sparkle errors
- Verify appcast.xml is accessible
- Check EdDSA signature is correct

### Download Fails
- Verify DMG URL in appcast.xml
- Check server logs for 404 errors
- Ensure DMG is in downloads directory

### Installation Fails
- Check code signing: `codesign --verify --deep Nottawa.app`
- Verify notarization: `spctl -a -t open --context context:primary-signature Nottawa.app`

## Current Status

✅ Sparkle framework wrapper created
✅ Server configured to host updates
✅ Appcast feed at https://nottawa.app/appcast.xml
✅ Downloads directory at https://nottawa.app/downloads/
✅ Release notes system in place
✅ Build and deployment scripts ready

⏳ Pending:
- Add Sparkle.framework to Xcode project
- Generate and configure EdDSA keys
- Update MainApp.cpp and MainStageView.cpp
- Configure code signing certificates

## Support

For issues with:
- Sparkle: https://sparkle-project.org/documentation/
- Build process: Check build_release.sh output
- Server: Check ~/dev/nottawa-server logs