#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  echo Build\ all\ projects
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
  echo Build\ all\ projects
fi

