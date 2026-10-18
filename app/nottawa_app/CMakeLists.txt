cmake_minimum_required(VERSION 3.21)
project(nottawa_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.8.0/msvc2022_64")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.8.0/msvc2022_64")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Network)
find_package(OF REQUIRED)

get_filename_component(NOTTAWA_REPO "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

# The engine is Nottawa.vcxproj built with /p:NottawaEngineLib=true (see
# scripts/patch-vcxproj.ps1). It must be built through the OF apps/myApps
# junction so the project's ..\..\..\addons paths resolve.
set(NOTTAWA_ENGINE_VCXPROJ "${OF_ROOT}/apps/myApps/Nottawa/Nottawa.vcxproj"
    CACHE FILEPATH "Nottawa.vcxproj opened through the OF apps/myApps junction")
set(NOTTAWA_ENGINE_LIB "${NOTTAWA_REPO}/obj/engine/x64/$<CONFIG>/NottawaEngine.lib")
add_custom_target(nottawa_engine
    COMMAND "${CMAKE_VS_MSBUILD_COMMAND}" "${NOTTAWA_ENGINE_VCXPROJ}"
            /p:NottawaEngineLib=true /p:Configuration=$<CONFIG> /p:Platform=x64
            /m /nologo /v:minimal
    COMMENT "Building NottawaEngine.lib"
    VERBATIM)

qt_standard_project_setup()

qt_add_executable(nottawa_app WIN32
    main.cpp
    MainWindow.h             MainWindow.cpp
    EngineBridge.h           EngineBridge.cpp
    StageWidget.h            StageWidget.cpp
    FramePresenter.h         FramePresenter.cpp
    PreviewWindow.h          PreviewWindow.cpp
    ShareStrandDialog.h      ShareStrandDialog.cpp
    ShaderCodeEditor.h       ShaderCodeEditor.cpp
    Icons.h                  Icons.cpp
    panels/PreviewPool.h     panels/PreviewPool.cpp
    panels/TileGridView.h    panels/TileGridView.cpp
    panels/StrandsPage.h     panels/StrandsPage.cpp
    panels/LibraryPage.h     panels/LibraryPage.cpp
    panels/CommunityPage.h   panels/CommunityPage.cpp
    panels/OscillatorsPage.h panels/OscillatorsPage.cpp
    ui/Controls.h            ui/Controls.cpp
    ui/ActionBar.h           ui/ActionBar.cpp
    ui/Toast.h               ui/Toast.cpp
    ui/AudioGraphs.h         ui/AudioGraphs.cpp
    panels/AudioPanel.h      panels/AudioPanel.cpp
    panels/StageView.h       panels/StageView.cpp
    SettingsDialog.h         SettingsDialog.cpp
    Screenshots.h            Screenshots.cpp
    inspector/ParameterRow.h inspector/ParameterRow.cpp
    inspector/PositionPad.h  inspector/PositionPad.cpp
    inspector/SourcePanels.h inspector/SourcePanels.cpp
    inspector/TransformPads.h inspector/TransformPads.cpp
    ui/WaveformPreview.h     ui/WaveformPreview.cpp
    nodes/NodeEditorView.h   nodes/NodeEditorView.cpp
    nodes/NodeItems.h        nodes/NodeItems.cpp
    Theme.h                  Theme.cpp
    CrashHandler.h           CrashHandler.cpp
    panels/BrowserPanel.h    panels/BrowserPanel.cpp
    panels/InspectorPanel.h  panels/InspectorPanel.cpp
    panels/LogPanel.h        panels/LogPanel.cpp
    panels/WelcomeOverlay.h  panels/WelcomeOverlay.cpp
)
add_dependencies(nottawa_app nottawa_engine)
target_include_directories(nottawa_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NOTTAWA_REPO}/src/Bridge
    ${NOTTAWA_REPO}/src/Engine)
target_link_directories(nottawa_app PRIVATE ${NOTTAWA_REPO}/libs/ffmpeg/lib)
# Same link inputs as Nottawa.exe (Nottawa.vcxproj AdditionalDependencies).
target_link_libraries(nottawa_app PRIVATE
    Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Network
    ${NOTTAWA_ENGINE_LIB}
    OF::OF
    avformat avcodec avutil swscale swresample
    winmm ole32 uuid avrt propsys shlwapi opengl32)
# The engine objects are compiled with /GL.
target_link_options(nottawa_app PRIVATE /LTCG)
# Symbols in a separate PDB (optimisation unchanged), so crash reports have
# readable stacks; /OPT keeps the exe as lean as without /DEBUG.
target_compile_options(nottawa_app PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(nottawa_app PRIVATE $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)

# OF's VS release libs use the dynamic CRT.
set_property(TARGET nottawa_app PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

get_target_property(_qmake Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin "${_qmake}" DIRECTORY)
file(GLOB _ffmpeg_dlls "${NOTTAWA_REPO}/bin/av*.dll" "${NOTTAWA_REPO}/bin/sw*.dll")
add_custom_command(TARGET nottawa_app POST_BUILD
    COMMAND "${_qt_bin}/windeployqt.exe" --no-translations --no-system-d3d-compiler --no-opengl-sw
            "$<TARGET_FILE:nottawa_app>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_ffmpeg_dlls} "$<TARGET_FILE_DIR:nottawa_app>"
    COMMENT "Deploying Qt and ffmpeg DLLs")
