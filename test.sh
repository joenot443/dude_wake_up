export CODE_SIGN_IDENTITY="9C3C3E7DA1B5BC8798DB8CDE81873AD7B94DC333"
export CODE_SIGN_ENTITLEMENTS="<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "https://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><dict><key>com.apple.security.app-sandbox</key><true/><key>com.apple.security.device.audio-input</key><true/><key>com.apple.security.device.camera</key><true/><key>com.apple.security.device.usb</key><true/><key>com.apple.security.files.user-selected.read-write</key><true/><key>com.apple.security.get-task-allow</key><true/><key>com.apple.security.network.client</key><true/><key>com.apple.security.network.server</key><true/></dict></plist>"

codesign --force --verbose --sign "${CODE_SIGN_IDENTITY}" --entitlements "${CODE_SIGN_ENTITLEMENTS}" "${ITEM}"
 


# ---- Code Sign App Package ----

# # WARNING: You may have to run Clean in Xcode after changing CODE_SIGN_IDENTITY!
# # Verify that $CODE_SIGN_IDENTITY is set
# if [ -z "${CODE_SIGN_IDENTITY}" ] ; then
# echo "CODE_SIGN_IDENTITY needs to be set for framework code-signing"
# exit 0
# fi

# if [ -z "${CODE_SIGN_ENTITLEMENTS}" ] ; then
# echo "CODE_SIGN_ENTITLEMENTS needs to be set for framework code-signing!"

# if [ "${CONFIGURATION}" = "Release" ] ; then
# exit 1
# else
# # Code-signing is optional for non-release builds.
# exit 0
# fi
# fi

# echo "${CODE_SIGN_ENTITLEMENTS}"


# ITEMS=""

# FRAMEWORKS_DIR="${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}"
# echo "$FRAMEWORKS_DIR"
# if [ -d "$FRAMEWORKS_DIR" ] ; then
# FRAMEWORKS=$(find "${FRAMEWORKS_DIR}" -depth -type d -name "*.framework" -or -name "*.dylib" -or -name "*.bundle" | sed -e "s/\(.*framework\)/\1\/Versions\/A\//")
# RESULT=$?
# if [[ $RESULT != 0 ]] ; then
# exit 1
# fi

# ITEMS="${FRAMEWORKS}"
# fi

# LOGINITEMS_DIR="${TARGET_BUILD_DIR}/${CONTENTS_FOLDER_PATH}/Library/LoginItems/"
# if [ -d "$LOGINITEMS_DIR" ] ; then
# LOGINITEMS=$(find "${LOGINITEMS_DIR}" -depth -type d -name "*.app")
# RESULT=$?
# if [[ $RESULT != 0 ]] ; then
# exit 1
# fi

# ITEMS="${ITEMS}"$'\n'"${LOGINITEMS}"
# fi

# # Prefer the expanded name, if available.
# CODE_SIGN_IDENTITY_FOR_ITEMS="${EXPANDED_CODE_SIGN_IDENTITY_NAME}"
# if [ "${CODE_SIGN_IDENTITY_FOR_ITEMS}" = "" ] ; then
# # Fall back to old behavior.
# CODE_SIGN_IDENTITY_FOR_ITEMS="${CODE_SIGN_IDENTITY}"
# fi

# echo "Identity:"
# echo "${CODE_SIGN_IDENTITY_FOR_ITEMS}"

# echo "Entitlements:"
# echo "${CODE_SIGN_ENTITLEMENTS}"

# echo "Found:"
# echo "${ITEMS}"

# # Change the Internal Field Separator (IFS) so that spaces in paths will not cause problems below.
# SAVED_IFS=$IFS
# IFS=$(echo -en "\n\b")

# # Loop through all items.
# for ITEM in $ITEMS;
# do
# echo "Stripping invalid archs '${ITEM}'"
# lipo -extract x86_64 "${ITEM}" -o "${ITEM}"
# echo "Signing '${ITEM}'"
# codesign --force --verbose --sign "${CODE_SIGN_IDENTITY_FOR_ITEMS}" --entitlements "${CODE_SIGN_ENTITLEMENTS}" "${ITEM}"
# RESULT=$?
# if [[ $RESULT != 0 ]] ; then
# echo "Failed to sign '${ITEM}'."
# IFS=$SAVED_IFS
# exit 1
# fi
# done

# # Restore $IFS.
# IFS=$SAVED_IFS

# fi
