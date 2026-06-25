cmake_minimum_required(VERSION 3.25)

project(Tide-island VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

find_package(OpenGL REQUIRED)
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)

find_package(Freetype REQUIRED)
find_package(harfbuzz REQUIRED)

set(SRC 
    main.cpp 
    source/island.cpp
    source/wayland.cpp
    source/wlr-layer-shell-unstable-v1-protocol.c 
    source/xdg-shell-client-protocol.c
    source/renderer.cpp
    source/environment.cpp
)

add_executable(Tide-island ${SRC})

target_compile_definitions(Tide-island PRIVATE SOKOL_GLES3)

target_include_directories(Tide-island PRIVATE 
    include/
    source/shaders/
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${WAYLAND_EGL_INCLUDE_DIRS}
)

target_link_libraries(Tide-island PRIVATE 
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_EGL_LIBRARIES}
    OpenGL::GL
    OpenGL::EGL
    Freetype::Freetype 
    harfbuzz::harfbuzz
)

target_compile_options(Tide-island PRIVATE -Wall -Wextra)

target_compile_options(Tide-island PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
)

target_compile_options(Tide-island PRIVATE
    -Wno-unused-but-set-variable
)
