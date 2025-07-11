clang++ \
-x c++ \
-target arm64-apple-macos10.15 \
-fmessage-length=0 \
-fdiagnostics-show-note-include-stack \
-fmacro-backtrace-limit=0 \
-fno-color-diagnostics \
-std=c++17 \
-stdlib=libc++ \
-Wno-trigraphs \
-fpascal-strings \
-O3 \
-fno-common \
-Wno-missing-field-initializers \
-Wno-missing-prototypes \
-Werror=return-type \
-Wunreachable-code \
-Wquoted-include-in-framework-header \
-Wno-non-virtual-dtor \
-Wno-overloaded-virtual \
-Wno-exit-time-destructors \
-Wno-missing-braces \
-Wparentheses \
-Wswitch \
-Wunused-function \
-Wno-unused-label \
-Wno-unused-parameter \
-Wno-unused-variable \
-Wno-unused-value \
-Wempty-body \
-Wuninitialized \
-Wno-unknown-pragmas \
-Wno-shadow \
-Wno-four-char-constants \
-Wno-conversion \
-Wconstant-conversion \
-Wint-conversion \
-Wbool-conversion \
-Wenum-conversion \
-Wno-float-conversion \
-Wnon-literal-null-conversion \
-Wobjc-literal-conversion \
-Wno-shorten-64-to-32 \
-Wno-newline-eof \
-Wno-c++11-extensions \
-Wno-implicit-fallthrough \
-DGL_SILENCE_DEPRECATION=1 \
-DGLES_SILENCE_DEPRECATION=1 \
-DCOREVIDEO_SILENCE_GL_DEPRECATION=1 \
-DGLM_FORCE_CTOR_INIT \
-DGLM_ENABLE_EXPERIMENTAL \
-DOF_NO_FMOD=1 \
-DCPPHTTPLIB_OPENSSL_SUPPORT=1 \
-DLINK_PLATFORM_MACOSX=1 \
-DASIO_HAS_VARIADIC_TEMPLATES=1 \
-DASIO_HAS_MOVE=1 \
-DASIO_STANDALONE=1 \
-D__MACOSX_CORE__ \
-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk \
-funroll-loops \
-fstrict-aliasing \
-Wdeprecated-declarations \
-Wno-invalid-offsetof \
-g \
-Wno-sign-conversion \
-Winfinite-recursion \
-Wmove \
-Wcomma \
-Wblock-capture-autoreleasing \
-Wstrict-prototypes \
-Wrange-loop-analysis \
-Wno-semicolon-before-method-body \
-I/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up/bin/include \
-I../../../libs/openFrameworks \
-I../../../libs/openFrameworks/3d \
-I../../../libs/openFrameworks/app \
-I../../../libs/openFrameworks/communication \
-I../../../libs/openFrameworks/events \
-I../../../libs/openFrameworks/gl \
-I../../../libs/openFrameworks/graphics \
-I../../../libs/openFrameworks/math \
-I../../../libs/openFrameworks/sound \
-I../../../libs/openFrameworks/types \
-I../../../libs/openFrameworks/utils \
-I../../../libs/openFrameworks/video \
-I../../../libs/openFrameworks/gl/shaders \
-I../../../libs/freetype/include \
-I../../../libs/freetype/include/freetype2 \
-I../../../libs/glew/include \
-I../../../libs/FreeImage/include \
-I../../../libs/tess2/include \
-I../../../libs/cairo/include \
-I../../../libs/rtAudio/include \
-I../../../libs/glfw/include \
-I../../../libs/utf8/include \
-I../../../libs/json/include \
-I../../../libs/glm/include \
-I../../../libs/curl/include \
-I../../../libs/openssl/include \
-I../../../libs/uriparser/include \
-I../../../libs/pugixml/include \
-I../../../libs/brotli/include \
-Isrc \
-Isrc/Apps \
-Isrc/Audio \
-Isrc/Feedback \
-Isrc/Models \
-Isrc/Network \
-Isrc/NodeLayout \
-Isrc/Oscillation \
-Isrc/Services \
-Isrc/Services/Commands \
-Isrc/Services/Midi \
-Isrc/Shading \
-Isrc/Util \
-Isrc/Util/observable \
-Isrc/Video \
-Isrc/Video/Recording \
-Isrc/Views \
-I../../../addons/ofxImGui/libs \
-I../../../addons/ofxImGui/libs/imgui \
-I../../../addons/ofxImGui/libs/imgui/backends \
-I../../../addons/ofxImGui/libs/imgui/docs \
-I../../../addons/ofxImGui/libs/imgui/extras \
-I../../../addons/ofxImGui/libs/imgui/src \
-I../../../addons/ofxImGui/src \
-I../../../addons/ofxMidi/libs \
-I../../../addons/ofxMidi/libs/pgmidi \
-I../../../addons/ofxMidi/libs/rtmidi \
-I../../../addons/ofxMidi/src \
-I../../../addons/ofxMidi/src/desktop \
-I../../../addons/ofxMidi/src/ios \
-I../../../addons/ofxSyphon/libs \
-I../../../addons/ofxSyphon/src \
-I../../../addons/ofxFastFboReader/src \
-Ilib/imgui-node-editor \
-Ilib/observable \
-Ilib/gist \
-Ilib/gist/src \
-Ilib/gist/src/libs \
-Ilib/gist/src/libs/kiss_fft130 \
-Ilib/implot \
-Ilib/ableton \
-Ilib/ableton/ableton \
-Ilib/ableton/ableton/asio \
-Ilib/ableton/ableton/discovery \
-Ilib/ableton/ableton/link \
-Ilib/ableton/ableton/platforms \
-Ilib/ableton/ableton/test \
-Ilib/ableton/ableton/util \
-Ilib/ableton/ableton/asio/detail \
-Ilib/ableton/ableton/asio/execution \
-Ilib/ableton/ableton/asio/experimental \
-Ilib/ableton/ableton/asio/generic \
-Ilib/ableton/ableton/asio/impl \
-Ilib/ableton/ableton/asio/ip \
-Ilib/ableton/ableton/asio/local \
-Ilib/ableton/ableton/asio/posix \
-Ilib/ableton/ableton/asio/ssl \
-Ilib/ableton/ableton/asio/traits \
-Ilib/ableton/ableton/asio/ts \
-Ilib/ableton/ableton/asio/windows \
-Ilib/ableton/ableton/discovery/test \
-Ilib/ableton/ableton/discovery/v1 \
-Ilib/ableton/ableton/link/v1 \
-Ilib/ableton/ableton/platforms/asio \
-Ilib/ableton/ableton/platforms/darwin \
-Ilib/ableton/ableton/platforms/esp32 \
-Ilib/ableton/ableton/platforms/linux \
-Ilib/ableton/ableton/platforms/posix \
-Ilib/ableton/ableton/platforms/stl \
-Ilib/ableton/ableton/platforms/windows \
-Ilib/ableton/ableton/test/serial_io \
-Ilib/ableton/ableton/util/test \
-Ilib/ableton/ableton/asio/detail/impl \
-Ilib/ableton/ableton/asio/execution/detail \
-Ilib/ableton/ableton/asio/execution/impl \
-Ilib/ableton/ableton/asio/experimental/detail \
-Ilib/ableton/ableton/asio/experimental/impl \
-Ilib/ableton/ableton/asio/generic/detail \
-Ilib/ableton/ableton/asio/ip/detail \
-Ilib/ableton/ableton/asio/ip/impl \
-Ilib/ableton/ableton/asio/local/detail \
-Ilib/ableton/ableton/asio/ssl/detail \
-Ilib/ableton/ableton/asio/ssl/impl \
-Ilib/ableton/ableton/asio/experimental/detail/impl \
-Ilib/ableton/ableton/asio/generic/detail/impl \
-Ilib/ableton/ableton/asio/ip/detail/impl \
-Ilib/ableton/ableton/asio/local/detail/impl \
-Ilib/ableton/ableton/asio/ssl/detail/impl \
-Ilib/ableton/asio \
-F/Users/jcrozier/open_frameworks/apps/myApps/dude_wake_up/bin \
src/main.cpp \
  -L../../../libs/openFrameworksCompiled/lib/osx \
-o main && ./main 