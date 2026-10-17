cmake_minimum_required(VERSION 3.21)
project(nottawa_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Prefer Qt via the standard install path.
if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.8.0/msvc2022_64")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.8.0/msvc2022_64")
endif()

# FindOF.cmake lives one level up.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(OF REQUIRED)

qt_standard_project_setup()

# Phase A.2 — same shell as A.1 plus a new OFStageEngine that links
# OpenFrameworks and renders through its primitives. Replaces the
# placeholder plasma stage with real OF-driven output.
qt_add_executable(nottawa_app
    main.cpp
    MainWindow.h          MainWindow.cpp
    StageWidget.h         StageWidget.cpp
    OFStageEngine.h       OFStageEngine.cpp
    Theme.h               Theme.cpp
    panels/BrowserPanel.h    panels/BrowserPanel.cpp
    panels/InspectorPanel.h  panels/InspectorPanel.cpp
    panels/LogPanel.h        panels/LogPanel.cpp
    panels/WelcomeOverlay.h  panels/WelcomeOverlay.cpp
)
target_include_directories(nottawa_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nottawa_app PRIVATE
    Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets
    OF::OF
    opengl32)

# OF is built with /MD (dynamic CRT) in VS release; match it.
set_property(TARGET nottawa_app PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

if(WIN32)
    add_custom_command(TARGET nottawa_app POST_BUILD
        COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
                --no-translations --no-system-d3d-compiler --no-opengl-sw
                "$<TARGET_FILE:nottawa_app>"
        COMMENT "windeployqt — copying Qt DLLs")
endif()
