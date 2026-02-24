#!/bin/sh
# Copy resources and frameworks for NottawaApp target

"$OF_PATH/scripts/osx/xcode_project.sh"
NOTTAWA_DIR="/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up"

mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/"
mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/"

# Copy Sparkle
SOURCE="/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up/lib/Sparkle.framework"
DEST="$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks"
if [ -d "$SOURCE" ]; then
    mkdir -p "$DEST"
    ditto "$SOURCE" "$DEST/Sparkle.framework"
    rm -rf "$DEST/Sparkle.framework/bin"
    codesign --force --sign - "$DEST/Sparkle.framework"
fi

# Copy resources into Resources/data
mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/"
cp -R "$NOTTAWA_DIR/images/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/images"
cp -R "$NOTTAWA_DIR/shaders/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/shaders"
cp -R "$NOTTAWA_DIR/markdown/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/markdown"
cp -R "$NOTTAWA_DIR/fonts/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/fonts"
cp -R "$NOTTAWA_DIR/misc/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/misc"
cp -R "$NOTTAWA_DIR/audio/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/audio"
