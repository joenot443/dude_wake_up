#!/bin/bash

"$OF_PATH/scripts/osx/xcode_project.sh"

mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/"
# Copy libfmod and change install directory for fmod to run
rsync -aved "$OF_PATH/libs/fmod/lib/osx/libfmod.dylib" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/";

rsync -aved "$NOTTAWA_DIR/lib/sentry-native/xcodebuild/Debug/libsentry.dylib" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/";

# Copy Sparkle framework to app bundle (excluding build tools)
if [ -d "$NOTTAWA_DIR/lib/Sparkle.framework" ]; then
    echo "Copying Sparkle.framework to app bundle..."
    # Use rsync with --exclude to skip the bin directory during copy
    rsync -aved --exclude="bin" "$NOTTAWA_DIR/lib/Sparkle.framework" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/"

    # Code sign the framework if code signing is enabled
    if [ -n "$CODE_SIGN_IDENTITY" ]; then
        echo "Code signing Sparkle.framework..."
        codesign --force --deep --sign "$CODE_SIGN_IDENTITY" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/Sparkle.framework"
    else
        # Use ad-hoc signing if no identity specified
        codesign --force --deep --sign - "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/Sparkle.framework"
    fi
fi

echo "$GCC_PREPROCESSOR_DEFINITIONS";

NOTTAWA_DIR="/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up"
alias

# Copy the shaders folder into Resources/data
mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/"

cp -R "$NOTTAWA_DIR/images/" "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/images"

cp -R "$NOTTAWA_DIR/shaders/"  "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/shaders"

# Copy the markdown folder into Resources/data
cp -R "$NOTTAWA_DIR/markdown/"  "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/markdown"

# Copy the fonts folder into Resources/data
cp -R "$NOTTAWA_DIR/fonts/"  "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/fonts"

# Copy the misc folder into Resources/data
cp -R "$NOTTAWA_DIR/misc/"  "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/misc"

# Copy the audio folder into Resources/data
cp -R "$NOTTAWA_DIR/audio/"  "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/audio"