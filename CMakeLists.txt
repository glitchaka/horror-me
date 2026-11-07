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

# Protected builds are deliberately opt-in; normal gameplay is unchanged.
# The native verifier is STATICALLY LINKED into the original HorrorMe.exe.
option(HORROR_ENABLE_DISK_SEAL "Require a signed original-USB installation" OFF)
set(HORROR_SEAL_PUBLIC_KEY_HEX "" CACHE STRING "Authority Ed25519 public key, 64 hex chars")
if(HORROR_ENABLE_DISK_SEAL)
    if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Disk-seal experiment requires MSVC x64 Windows.")
    endif()
    string(LENGTH "${HORROR_SEAL_PUBLIC_KEY_HEX}" SEAL_KEY_LENGTH)
    if(NOT SEAL_KEY_LENGTH EQUAL 64 OR NOT HORROR_SEAL_PUBLIC_KEY_HEX MATCHES "^[0-9A-Fa-f]+$")
        message(FATAL_ERROR "Provide HORROR_SEAL_PUBLIC_KEY_HEX (64 hexadecimal digits).")
    endif()
    find_program(CARGO_EXE cargo REQUIRED)
    set(SEAL_RUST_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/security/seal-bridge/Cargo.toml")
    set(SEAL_RUST_TARGET "${CMAKE_CURRENT_BINARY_DIR}/seal-cargo")
    set(SEAL_RUST_LIB "${SEAL_RUST_TARGET}/x86_64-pc-windows-msvc/release/horror_seal_bridge.lib")
    add_custom_command(
        OUTPUT "${SEAL_RUST_LIB}"
        COMMAND "${CMAKE_COMMAND}" -E env
            "CARGO_TARGET_DIR=${SEAL_RUST_TARGET}"
            "RUSTFLAGS=-C target-feature=+crt-static"
            "${CARGO_EXE}" build --release
                --manifest-path "${SEAL_RUST_MANIFEST}" --target x86_64-pc-windows-msvc
        DEPENDS
            "${SEAL_RUST_MANIFEST}"
            "${CMAKE_CURRENT_SOURCE_DIR}/security/seal-bridge/src/lib.rs"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building statically linked disk-seal Rust verification"
        VERBATIM
    )
    add_custom_target(HorrorSealRust DEPENDS "${SEAL_RUST_LIB}")
    add_dependencies(HorrorMe HorrorSealRust)
    target_sources(HorrorMe PRIVATE src/security/InstallationGuard.cpp)
    target_compile_definitions(HorrorMe PRIVATE
        HORROR_ENABLE_DISK_SEAL=1
        "HORROR_SEAL_PUBLIC_KEY_HEX=\"${HORROR_SEAL_PUBLIC_KEY_HEX}\""
    )
    target_link_libraries(HorrorMe PRIVATE
        "${SEAL_RUST_LIB}"
        advapi32 bcrypt crypt32 userenv ws2_32 secur32 ntdll
        shell32 ole32 oleaut32
    )
endif()

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
