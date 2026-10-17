cmake_minimum_required(VERSION 3.21)
project(nottawa_shell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.8.0/msvc2022_64")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.8.0/msvc2022_64")
endif()

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

qt_standard_project_setup()

# Phase A.1 — shell only. Stage uses placeholder plasma engine; the
# real Nottawa engine integration lands in Phase A.2.
qt_add_executable(nottawa_shell
    main.cpp
    MainWindow.h          MainWindow.cpp
    StageWidget.h         StageWidget.cpp
    StageEngine.h         StageEngine.cpp
    Theme.h               Theme.cpp
    panels/BrowserPanel.h    panels/BrowserPanel.cpp
    panels/InspectorPanel.h  panels/InspectorPanel.cpp
    panels/LogPanel.h        panels/LogPanel.cpp
    panels/WelcomeOverlay.h  panels/WelcomeOverlay.cpp
)
target_include_directories(nottawa_shell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nottawa_shell PRIVATE
    Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

if(WIN32)
    add_custom_command(TARGET nottawa_shell POST_BUILD
        COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
                --no-translations --no-system-d3d-compiler --no-opengl-sw
                "$<TARGET_FILE:nottawa_shell>"
        COMMENT "windeployqt — copying Qt DLLs")
endif()
