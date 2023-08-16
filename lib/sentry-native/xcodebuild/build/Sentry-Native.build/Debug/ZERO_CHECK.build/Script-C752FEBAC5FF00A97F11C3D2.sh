#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  make -f /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  make -f /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  make -f /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  make -f /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/CMakeScripts/ReRunCMake.make
fi

