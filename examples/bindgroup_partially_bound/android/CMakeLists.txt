# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.18.1)

file(TO_CMAKE_PATH "${ANDROID_NDK}" ANDROID_NDK_PATH)

set(APP_GLUE_DIR ${ANDROID_NDK_PATH}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library(
    app-glue STATIC
    ${APP_GLUE_DIR}/android_native_app_glue.c
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(KDGpu::KDGpu QUIET)
if(NOT TARGET KDGpu::KDGpu)
    message(STATUS "KDGpu was not found. Fetching from git")
    set(FETCHCONTENT_SOURCE_DIR_KDGPU ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
    FetchContent_Declare(
        KDGpu
        GIT_REPOSITORY https://github.com/KDAB/KDGpu.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(KDGpu)
endif()

project("bindgroup_partially_bound")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_FIND_DEBUG_MODE TRUE)

add_library(
    # Sets the name of the library.
    bindgroup_partially_bound
    # Sets the library as a shared library.
    SHARED
    # Provides a relative path to your source file(s).
    ../bindgroup_partially_bound.cpp ../main.cpp
)

target_include_directories(
    bindgroup_partially_bound
    PRIVATE "../" "${CMAKE_BINARY_DIR}/_deps/glm-src" "${CMAKE_BINARY_DIR}/_deps/stb-src"
)

target_compile_definitions(bindgroup_partially_bound PRIVATE PLATFORM_ANDROID)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DVK_USE_PLATFORM_ANDROID_KHR")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

target_link_libraries(
    # Specifies the target library.
    bindgroup_partially_bound
    vulkan
    nativewindow
    app-glue
    log
    android
    KDFoundation
    KDGui
    KDUtils
    KDGpu
    imgui
    KDGpuKDGui
    KDGpuExample
    KDGpuExampleResources
    KDGpuExampleShaderResources
    KDXr
    spdlog
    openxr_loader
)
