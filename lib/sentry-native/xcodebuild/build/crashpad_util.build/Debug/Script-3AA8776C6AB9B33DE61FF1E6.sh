#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util
  /opt/homebrew/bin/python3.11 /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/mig.py --arch FROM_ENV --sdk $SDKROOT --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/.. --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/../compat/mac /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/child_port.defs /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portUser.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_port.h /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.h
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util
  /opt/homebrew/bin/python3.11 /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/mig.py --arch FROM_ENV --sdk $SDKROOT --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/.. --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/../compat/mac /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/child_port.defs /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portUser.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_port.h /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.h
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util
  /opt/homebrew/bin/python3.11 /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/mig.py --arch FROM_ENV --sdk $SDKROOT --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/.. --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/../compat/mac /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/child_port.defs /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portUser.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_port.h /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.h
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util
  /opt/homebrew/bin/python3.11 /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/mig.py --arch FROM_ENV --sdk $SDKROOT --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/.. --include=/Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/../compat/mac /Users/jcrozier/Downloads/sentry-nativeee/external/crashpad/util/mach/child_port.defs /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portUser.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.c /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_port.h /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/crashpad_build/util/util/mach/child_portServer.h
fi

