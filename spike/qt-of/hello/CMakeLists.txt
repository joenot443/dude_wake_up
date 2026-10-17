cmake_minimum_required(VERSION 3.21)
project(nottawa_qt_hello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt is installed via aqtinstall at C:\Qt\6.8.0\msvc2022_64.
# Point CMake at the Qt6 config either via the Qt6_DIR env var or by
# including the standard Qt install prefix.
if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt/6.8.0/msvc2022_64")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.8.0/msvc2022_64")
endif()

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(hello WIN32 main.cpp)
target_link_libraries(hello PRIVATE Qt6::Widgets)

# Deploy Qt DLLs next to the exe (windeployqt). Only runs if the
# executable actually built — avoids chasing errors during config.
if(WIN32)
    add_custom_command(TARGET hello POST_BUILD
        COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
                --no-translations --no-system-d3d-compiler --no-opengl-sw
                "$<TARGET_FILE:hello>"
        COMMENT "windeployqt — copying Qt DLLs next to hello.exe")
endif()
