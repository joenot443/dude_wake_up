# FindOF.cmake — locates OpenFrameworks 0.12.1 (VS release build) and
# exposes it as an IMPORTED INTERFACE target `OF::OF`.
#
# Mirrors the config in Nottawa.vcxproj →
#   libs/openFrameworksCompiled/project/vs/openFrameworksRelease.props
#   libs/openFrameworksCompiled/project/vs/openFrameworksCommon.props
#
# Usage:
#   list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
#   find_package(OF REQUIRED)
#   target_link_libraries(myapp PRIVATE OF::OF)
#
# Env/variable OF_ROOT selects the OF install; defaults to C:/OF/of_v0.12.1_vs_64_release.

if(DEFINED ENV{OF_ROOT})
    set(OF_ROOT "$ENV{OF_ROOT}" CACHE PATH "OpenFrameworks install root")
else()
    set(OF_ROOT "C:/OF/of_v0.12.1_vs_64_release" CACHE PATH "OpenFrameworks install root")
endif()

if(NOT EXISTS "${OF_ROOT}/libs/openFrameworks/ofMain.h")
    message(FATAL_ERROR
        "FindOF: ofMain.h not found under '${OF_ROOT}/libs/openFrameworks'. "
        "Set OF_ROOT to your OpenFrameworks 0.12.1 install.")
endif()

find_file(OF_LIB_RELEASE openframeworksLib.lib
    PATHS "${OF_ROOT}/libs/openFrameworksCompiled/lib/vs/x64"
    NO_DEFAULT_PATH REQUIRED)

# Include dirs — order matters only for internal OF overrides, otherwise
# a flat list is fine.
set(_OF_INCLUDES
    "${OF_ROOT}/libs/openFrameworks"
    "${OF_ROOT}/libs/openFrameworks/graphics"
    "${OF_ROOT}/libs/openFrameworks/app"
    "${OF_ROOT}/libs/openFrameworks/sound"
    "${OF_ROOT}/libs/openFrameworks/utils"
    "${OF_ROOT}/libs/openFrameworks/communication"
    "${OF_ROOT}/libs/openFrameworks/video"
    "${OF_ROOT}/libs/openFrameworks/types"
    "${OF_ROOT}/libs/openFrameworks/math"
    "${OF_ROOT}/libs/openFrameworks/3d"
    "${OF_ROOT}/libs/openFrameworks/gl"
    "${OF_ROOT}/libs/openFrameworks/events"
    "${OF_ROOT}/libs/glm/include"
    "${OF_ROOT}/libs/rtAudio/include"
    "${OF_ROOT}/libs/freetype/include"
    # freetype2 subdir listed in OF's .props but not present in this
    # install; freetype/include is enough for ft2build.h.
    "${OF_ROOT}/libs/FreeImage/include"
    "${OF_ROOT}/libs/videoInput/include"
    "${OF_ROOT}/libs/glew/include"
    # glu lives in Windows SDK, not OF; its headers come with Visual Studio.
    "${OF_ROOT}/libs/tess2/include"
    "${OF_ROOT}/libs/cairo/include"
    "${OF_ROOT}/libs/pixman/include"
    "${OF_ROOT}/libs/libpng/include"
    "${OF_ROOT}/libs/zlib/include"
    "${OF_ROOT}/libs/glfw/include"
    "${OF_ROOT}/libs/openssl/include"
    "${OF_ROOT}/libs/utf8/include"
    "${OF_ROOT}/libs/json/include"
    "${OF_ROOT}/libs/curl/include"
    "${OF_ROOT}/libs/uriparser/include"
    "${OF_ROOT}/libs/pugixml/include"
    "${OF_ROOT}/addons"
)

# Lib search dirs per the OF .props file.
set(_OF_LIBDIRS
    "${OF_ROOT}/libs/openFrameworksCompiled/lib/vs/x64"
    "${OF_ROOT}/libs/glfw/lib/vs/x64"
    "${OF_ROOT}/libs/rtAudio/lib/vs/x64"
    "${OF_ROOT}/libs/FreeImage/lib/vs/x64"
    "${OF_ROOT}/libs/brotli/lib/vs/x64"
    "${OF_ROOT}/libs/freetype/lib/vs/x64"
    "${OF_ROOT}/libs/videoInput/lib/vs/x64"
    "${OF_ROOT}/libs/cairo/lib/vs/x64"
    "${OF_ROOT}/libs/pixman/lib/vs/x64"
    "${OF_ROOT}/libs/libpng/lib/vs/x64"
    "${OF_ROOT}/libs/zlib/lib/vs/x64"
    "${OF_ROOT}/libs/glew/lib/vs/x64"
    "${OF_ROOT}/libs/glu/lib/vs/x64"
    "${OF_ROOT}/libs/openssl/lib/vs/x64"
    "${OF_ROOT}/libs/curl/lib/vs/x64"
    "${OF_ROOT}/libs/tess2/lib/vs/x64"
    "${OF_ROOT}/libs/uriparser/lib/vs/x64"
    "${OF_ROOT}/libs/pugixml/lib/vs/x64"
)

# All the libs OF links against on Windows release build.
set(_OF_LIBS
    openframeworksLib
    libcairo libpixman-1 libpng zlib
    brotlicommon brotlidec brotlienc
    msimg32 OpenGL32 GLu32
    kernel32 setupapi Vfw32 comctl32
    rtAudio videoInput freetype FreeImage dsound
    user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid
    libglew32 libssl libcrypto crypt32 libcurl
    uriparser pugixml Ws2_32 tess2 glfw3 winmm
    # From openFrameworksCommon.props extras:
    mf mfplat mfuuid d3d11 mfreadwrite xaudio2 odbc32 odbccp32 wldap32
)

set(_OF_DEFINES
    WIN32 NDEBUG _CONSOLE
    POCO_STATIC CAIRO_WIN32_STATIC_BUILD
    DISABLE_SOME_FLOATING_POINT URI_STATIC_BUILD
    GLM_FORCE_CTOR_INIT GLM_ENABLE_EXPERIMENTAL
    NOMINMAX WIN32_LEAN_AND_MEAN _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS
    __WINDOWS_WASAPI__ __WINDOWS_MM__ TARGET_WIN32
    OF_NO_FMOD=1 USE_KISS_FFT
    JSON_DIAGNOSTICS=1
)

add_library(OF::OF INTERFACE IMPORTED)
set_target_properties(OF::OF PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_OF_INCLUDES}"
    INTERFACE_COMPILE_DEFINITIONS "${_OF_DEFINES}"
    INTERFACE_COMPILE_FEATURES    "cxx_std_17"
)
# Link dirs via a generator expression so they appear on the link line
# only when consumers link against this target.
target_link_directories(OF::OF INTERFACE ${_OF_LIBDIRS})
target_link_libraries(OF::OF INTERFACE ${_OF_LIBS})

# OF's build drops atlthunk/LIBC/LIBCMT per openFrameworksRelease.props.
# Mirror that so our link line stays consistent with how OF itself was built.
target_link_options(OF::OF INTERFACE
    "/NODEFAULTLIB:atlthunk.lib"
    "/NODEFAULTLIB:LIBC.lib"
    "/NODEFAULTLIB:LIBCMT"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OF
    REQUIRED_VARS OF_ROOT OF_LIB_RELEASE)

message(STATUS "FindOF: using ${OF_ROOT}")
