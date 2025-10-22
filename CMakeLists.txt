# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.16)

option(KDGPU_USE_EXTERNAL_DEPENDENCIES
       "Assume all project dependencies are available on the system. Don't fetch through vcpkg." OFF
)

# vcpkg manifest mode
if(NOT KDGPU_USE_EXTERNAL_DEPENDENCIES)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        include(cmake/KDGpuFunctions.cmake)
        kdgpu_check_submodule_exists(vcpkg vcpkg)
        set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
    endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
cmake_policy(SET CMP0090 NEW) # Stop export(PACKAGE) from modifying the system-wide cmake package system
cmake_policy(SET CMP0117 NEW) # Do not add /GR to CMAKE_CXX_FLAGS

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

include(FeatureSummary)
include(CMakeDependentOption)

option(KDGPU_BUILD_SHARED_LIBS "Build shared libraries" ON)
set(BUILD_SHARED_LIBS ${KDGPU_BUILD_SHARED_LIBS})

option(KDGPU_CODE_COVERAGE "Code Coverage" OFF)
option(KDGPU_BUILD_TESTS "Build tests" ON)
option(KDGPU_BUILD_KDGPUKDGUI "Build KDGpuKDGui" ON)
option(KDGPU_BUILD_KDGPUUTILS "Build KDGpuUtils" ON)
cmake_dependent_option(
    KDGPU_BUILD_KDGPUEXAMPLE
    "Build KDGpuExample"
    ON
    KDGPU_BUILD_KDGPUKDGUI
    OFF
)
option(KDGPU_BUILD_EXAMPLES "Build examples" ON)
cmake_dependent_option(
    KDGPU_BUILD_ASSETS
    "Build Assets"
    ON
    KDGPU_BUILD_EXAMPLES
    OFF
)
option(KDGPU_HLSL_SUPPORT "Try to find and use the dxc HLSL to SPIR-V compiler" OFF)

add_feature_info(KDGpuKDGui ${KDGPU_BUILD_KDGPUKDGUI} "Build KDGpuKDGui")
add_feature_info(KDGpuExample ${KDGPU_BUILD_KDGPUEXAMPLE} "Build KDGpuExample")

option(KDGPU_BUILD_KDXR "Build KDXr" ON)
add_feature_info(OpenXR ${KDGPU_BUILD_KDXR} "Enable support for OpenXR")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(KDGPU_CODE_COVERAGE)
        include(cmake/CodeCoverage.cmake)
    endif()
endif()

# vcpkg conditional features
if(KDGPU_BUILD_EXAMPLES)
    list(APPEND VCPKG_MANIFEST_FEATURES "examples")
endif()

if(KDGPU_BUILD_KDGPUKDGUI)
    list(APPEND VCPKG_MANIFEST_FEATURES "kdgpukdgui")
endif()

if(KDGPU_BUILD_KDGPUUTILS)
    list(APPEND VCPKG_MANIFEST_FEATURES "kdgpuutils")
endif()

if(KDGPU_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "testing")
endif()

if(KDGPU_HLSL_SUPPORT)
    list(APPEND VCPKG_MANIFEST_FEATURES "hlsl")
endif()

if(KDGPU_BUILD_KDXR)
    list(APPEND VCPKG_MANIFEST_FEATURES "openxr")
endif()

message(STATUS "Vcpkg manifest features: ${VCPKG_MANIFEST_FEATURES}")

# Register Project once all vcpkg features have been set
project(
    KDGpu
    LANGUAGES CXX
    VERSION 0.9.3
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckAtomic)
include(GNUInstallDirs)
include(KDGpuCompileShader)
include(ECMEnableSanitizers)

# Note: Set before including dependencies.cmake, so 3rd party libs end up in the correct dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Package Dependencies
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

# Add sources
add_subdirectory(src)

# Feature Summary
add_feature_info(KDGpu-Assets ${KDGPU_BUILD_ASSETS} "Build Assets")
if(KDGPU_BUILD_ASSETS)
    add_subdirectory(assets)
endif()

add_feature_info(KDGpu-Tests ${KDGPU_BUILD_TESTS} "Build Tests")
if(KDGPU_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_feature_info(KDGpu-Examples ${KDGPU_BUILD_EXAMPLES} "Build Examples")
if(KDGPU_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

option(KDGPU_DOCS "Build the API documentation" OFF)

add_feature_info(KDGpu-Documentation ${KDGPU_DOCS} "Build Documentation")
if(KDGPU_DOCS)
    add_subdirectory(docs)
endif()

feature_summary(WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND ENABLED_FEATURES DISABLED_FEATURES INCLUDE_QUIET_PACKAGES)
