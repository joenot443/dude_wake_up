# Sparkle Framework and Tools

## Location
The Sparkle framework and its associated tools are located in:
```
lib/Sparkle.framework/
├── bin/                    # Command-line tools
│   ├── generate_keys       # Generate EdDSA keys for signing
│   ├── sign_update        # Sign updates for distribution
│   ├── generate_appcast   # Generate appcast feed from releases
│   └── BinaryDelta        # Create delta updates
└── [framework contents]    # Framework for app integration
```

## Tool Usage

### Generate Keys (First Time Setup)
```bash
./lib/Sparkle.framework/bin/generate_keys
```
- Generates EdDSA public/private key pair
- Private key stored in macOS Keychain (account: ed25519)
- Public key: `2ISmClcG8GYubpCKqQpOqRMOYqHmQKOjo3XOjW9z7Nk=` (already in Info.plist)

### Sign Updates
```bash
./lib/Sparkle.framework/bin/sign_update [DMG file] --account ed25519 -p
```
- Signs DMG files for distribution
- Uses key from Keychain (no need to specify private key)
- Returns EdDSA signature for appcast.xml

### Generate Appcast
```bash
./lib/Sparkle.framework/bin/generate_appcast [releases directory]
```
- Generates appcast.xml from a directory of releases
- Automatically includes signatures and file sizes

## Important Notes

1. **DO NOT** copy these tools to the app bundle - they're build tools only
2. **DO NOT** commit private keys to git
3. The `bin/` directory is excluded when embedding Sparkle.framework in the app
4. All scripts use relative paths: `./lib/Sparkle.framework/bin/[tool]`

## Related Scripts

- `build_release.sh` - Uses `sign_update` to sign releases
- `deploy_update.sh` - Uses `sign_update` to sign for deployment
- Build script in Xcode - Excludes `bin/` when copying framework