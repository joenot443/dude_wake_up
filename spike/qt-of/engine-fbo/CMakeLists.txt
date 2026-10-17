cmake_minimum_required(VERSION 3.21)
project(nottawa_qt_engine_fbo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.8.0/msvc2022_64")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.8.0/msvc2022_64")
endif()

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

qt_standard_project_setup()

# Spike build — leave WIN32 OFF so qInfo/qWarning reach a console.
# For the real app we'd put it back on.
qt_add_executable(engine_fbo
    main.cpp
    EngineWidget.h  EngineWidget.cpp
    EngineThread.h  EngineThread.cpp
)
target_link_libraries(engine_fbo PRIVATE
    Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)

if(WIN32)
    add_custom_command(TARGET engine_fbo POST_BUILD
        COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
                --no-translations --no-system-d3d-compiler --no-opengl-sw
                "$<TARGET_FILE:engine_fbo>"
        COMMENT "windeployqt — copying Qt DLLs")
endif()
