# CMake generated Testfile for 
# Source directory: /Users/jcrozier/Downloads/sentry-nativeee
# Build directory: /Users/jcrozier/Downloads/sentry-nativeee/xcodebuild
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(sentry_example "/Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Debug/sentry_example")
  set_tests_properties(sentry_example PROPERTIES  _BACKTRACE_TRIPLES "/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;597;add_test;/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(sentry_example "/Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/Release/sentry_example")
  set_tests_properties(sentry_example PROPERTIES  _BACKTRACE_TRIPLES "/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;597;add_test;/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(sentry_example "/Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/MinSizeRel/sentry_example")
  set_tests_properties(sentry_example PROPERTIES  _BACKTRACE_TRIPLES "/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;597;add_test;/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(sentry_example "/Users/jcrozier/Downloads/sentry-nativeee/xcodebuild/RelWithDebInfo/sentry_example")
  set_tests_properties(sentry_example PROPERTIES  _BACKTRACE_TRIPLES "/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;597;add_test;/Users/jcrozier/Downloads/sentry-nativeee/CMakeLists.txt;0;")
else()
  add_test(sentry_example NOT_AVAILABLE)
endif()
subdirs("src")
subdirs("crashpad_build")
subdirs("tests/unit")
