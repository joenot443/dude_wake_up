#!/bin/sh
# Copy resources and frameworks for NottawaApp target

"$OF_PATH/scripts/osx/xcode_project.sh"
NOTTAWA_DIR="/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up"

mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/"

# Copy resources into Resources/data
mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/"
cp -R "$NOTTAWA_DIR/images/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/images"
cp -R "$NOTTAWA_DIR/shaders/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/shaders"
cp -R "$NOTTAWA_DIR/markdown/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/markdown"
cp -R "$NOTTAWA_DIR/fonts/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/fonts"
cp -R "$NOTTAWA_DIR/misc/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/misc"
cp -R "$NOTTAWA_DIR/audio/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/audio"

# Strip extended attributes from the bundle. macOS slaps `com.apple.quarantine`
# on anything saved from a download/browser; App Store Connect rejects any
# binary that ships a single file with that attribute (ITMS-91109). Cleaning
# every xattr here, post-copy and pre-codesign, prevents the rejection from
# ever happening — even if a future asset gets dragged in via the browser.
xattr -cr "$TARGET_BUILD_DIR/$PRODUCT_NAME.app"
