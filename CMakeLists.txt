cmake_minimum_required(VERSION 3.21)
project(HorrorMe VERSION 0.1.0 LANGUAGES CXX)

add_executable(HorrorMe WIN32
    src/platform/WinApp.cpp
    src/platform/WinAudio.cpp
    src/core/Maze.cpp
    src/core/Session.cpp
    src/render/Canvas.cpp
    src/render/SceneRenderer.cpp
    src/render/HudRenderer.cpp
)

target_compile_features(HorrorMe PRIVATE cxx_std_20)
set_target_properties(HorrorMe PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(HorrorMe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT WIN32)
    message(FATAL_ERROR "Esta implementación utiliza Win32 y WinMM: compilar para Windows.")
endif()

target_compile_definitions(HorrorMe PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(HorrorMe PRIVATE user32 gdi32 winmm)

if(MSVC)
    target_compile_options(HorrorMe PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(HorrorMe PRIVATE -Wall -Wextra -Wpedantic)
endif()
