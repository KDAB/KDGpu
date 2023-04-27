# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
cmake_policy(SET CMP0090 NEW) # Stop export(PACKAGE) from modifying the system-wide cmake package system
cmake_policy(SET CMP0117 NEW) # Do not add /GR to CMAKE_CXX_FLAGS

project(KDGpu VERSION 0.0.1)

set(BUILD_SHARED_LIBS ON)
add_definitions(-DKDGPU_ASSET_PATH="${CMAKE_CURRENT_BINARY_DIR}/assets")

if(KDGPU_LARGE_ASSETS)
    add_definitions(-DKDGPU_LARGE_ASSET_PATH="${CMAKE_CURRENT_BINARY_DIR}/assets/large-assets")
endif()

# Add a DEBUG define on DEBUG builds
add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>")

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APPLE)
   OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT APPLE)
   OR (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND NOT WIN32)
)
    # Linker warnings should be treated as errors
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings ${CMAKE_MODULE_LINKER_FLAGS}")

    # Do not allow undefined symbols, even in non-symbolic shared libraries
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_MODULE_LINKER_FLAGS}")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    add_compile_options(-Wimplicit-fallthrough)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # cmake-lint: disable=C0301
    # Silences a warning on raspberry pi gcc. See:
    # https://stackoverflow.com/questions/48149323/what-does-the-gcc-warning-project-parameter-passing-for-x-changed-in-gcc-7-1-m
    add_compile_options(-Wno-psabi)
endif()

include(CompileShader)
include(CheckAtomic)
include(ECMEnableSanitizers)
include(FeatureSummary)
include(CMakeDependentOption)
include(GNUInstallDirs)

# Note: Set before including dependencies.cmake, so 3rd party libs end up in the correct dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(KDGPU_CODE_COVERAGE "Code Coverage" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(KDGPU_CODE_COVERAGE)
        include(cmake/CodeCoverage.cmake)
    endif()
endif()

add_subdirectory(src)
add_subdirectory(assets)

option(KDGPU_BUILD_TESTS "Build tests" ON)

if(KDGPU_BUILD_TESTS)
    add_feature_info(KDGpu-Tests ON "Build Tests")
    enable_testing()
    add_subdirectory(tests)
endif()

option(KDGPU_BUILD_EXAMPLES "Build examples" ON)

if(KDGPU_BUILD_EXAMPLES)
    add_feature_info(KDGpu-Examples ON "Build Examples")
    add_subdirectory(examples)
endif()

feature_summary(WHAT PACKAGES_FOUND ENABLED_FEATURES PACKAGES_NOT_FOUND DISABLED_FEATURES INCLUDE_QUIET_PACKAGES)
