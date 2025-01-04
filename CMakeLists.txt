cmake_minimum_required(VERSION 3.29.2)

project(LunaticVibesF VERSION 0.24.0 LANGUAGES CXX)
set(PROJECT_TITLE "Lunatic Vibes F")

configure_file(
    "${CMAKE_SOURCE_DIR}/src/cmake_config.h.in"
    "${PROJECT_BINARY_DIR}/cmake_config.h"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

if(MSVC)
    add_compile_options("/utf-8")
    add_compile_options("/bigobj")
    add_definitions(-D_WIN32_WINNT=0x0601) # Target Windows 7
endif()

if(WIN32)
    add_definitions(-DNOMINMAX)
endif()

set(GRAPHICS_BACKEND SDL2 CACHE STRING "Graphics backend")
if(${GRAPHICS_BACKEND} STREQUAL SDL2)
    add_definitions(-DRENDER_SDL2)
endif()

add_subdirectory(ext)

set(PROJECT_INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/src
    CACHE INTERNAL
    "include directory"
)
list(APPEND PROJECT_INCLUDE_DIR ${PROJECT_BINARY_DIR})
add_subdirectory(src)

option(BUILD_TEST "Enable building gtest targets" YES)
if(${BUILD_TEST})
    enable_testing()
    add_subdirectory(test)
endif()

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
include(CPack)
