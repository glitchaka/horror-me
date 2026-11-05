cmake_minimum_required(VERSION 3.22.1)
project(HorrorMeAndroid LANGUAGES CXX)

set(GAME_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../")
add_library(horrorme SHARED
    NativeBridge.cpp
    "${GAME_ROOT}/src/core/Maze.cpp"
    "${GAME_ROOT}/src/core/RevisorAI.cpp"
    "${GAME_ROOT}/src/core/Session.cpp"
    "${GAME_ROOT}/src/render/Canvas.cpp"
    "${GAME_ROOT}/src/render/SceneRenderer.cpp"
    "${GAME_ROOT}/src/render/HudRenderer.cpp"
)

target_include_directories(horrorme PRIVATE "${GAME_ROOT}/include")
target_compile_features(horrorme PRIVATE cxx_std_20)
target_compile_options(horrorme PRIVATE -Wall -Wextra -Wpedantic -O2)
target_link_options(horrorme PRIVATE "-Wl,-z,max-page-size=16384")
