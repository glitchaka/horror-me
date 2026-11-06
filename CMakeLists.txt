cmake_minimum_required(VERSION 3.21)
project(HorrorMe VERSION 0.4.1 LANGUAGES CXX)

# Statically link the MSVC runtime: the released .exe does not require
# a separate Visual C++ Redistributable installation.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_executable(HorrorMe WIN32
    src/platform/WinApp.cpp
    src/platform/WinAudio.cpp
    src/core/Maze.cpp
    src/core/RevisorAI.cpp
    src/core/Session.cpp
    src/render/Canvas.cpp
    src/render/FontAtlas.cpp
    src/render/MaterialArtist.cpp
    src/render/Atmosphere.cpp
    src/render/PixelPresenter.cpp
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

include(CTest)
if(BUILD_TESTING)
    add_executable(HorrorCoreTests
        tests/CoreTests.cpp
        src/core/Maze.cpp
        src/core/RevisorAI.cpp
        src/core/Session.cpp
    )
    target_compile_features(HorrorCoreTests PRIVATE cxx_std_20)
    set_target_properties(HorrorCoreTests PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(HorrorCoreTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(MSVC)
        target_compile_options(HorrorCoreTests PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(HorrorCoreTests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME HorrorCoreTests COMMAND HorrorCoreTests)
endif()

# Headless checks of procedural artwork and the baked Annapurna typography.
if(BUILD_TESTING)
    add_executable(HorrorRenderTests
        tests/RenderTests.cpp
        src/core/Maze.cpp
        src/core/RevisorAI.cpp
        src/core/Session.cpp
        src/render/Canvas.cpp
        src/render/FontAtlas.cpp
        src/render/MaterialArtist.cpp
        src/render/Atmosphere.cpp
        src/render/PixelPresenter.cpp
        src/render/SceneRenderer.cpp
        src/render/HudRenderer.cpp
    )
    target_compile_features(HorrorRenderTests PRIVATE cxx_std_20)
    set_target_properties(HorrorRenderTests PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(HorrorRenderTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(MSVC)
        target_compile_options(HorrorRenderTests PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(HorrorRenderTests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME HorrorRenderTests COMMAND HorrorRenderTests)
endif()
