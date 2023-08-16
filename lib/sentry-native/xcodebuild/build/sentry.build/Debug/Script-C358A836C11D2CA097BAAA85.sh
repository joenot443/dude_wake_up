#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  /opt/homebrew/Cellar/cmake/3.25.2/bin/cmake -E cmake_symlink_library /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Debug/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Debug/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Debug/libsentry.dylib
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  /opt/homebrew/Cellar/cmake/3.25.2/bin/cmake -E cmake_symlink_library /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Release/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Release/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Release/libsentry.dylib
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  /opt/homebrew/Cellar/cmake/3.25.2/bin/cmake -E cmake_symlink_library /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/MinSizeRel/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/MinSizeRel/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/MinSizeRel/libsentry.dylib
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  /opt/homebrew/Cellar/cmake/3.25.2/bin/cmake -E cmake_symlink_library /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/RelWithDebInfo/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/RelWithDebInfo/libsentry.dylib /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/RelWithDebInfo/libsentry.dylib
fi

