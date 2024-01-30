# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
cmake_minimum_required(VERSION 3.18.1)

file(TO_CMAKE_PATH "${ANDROID_NDK}" ANDROID_NDK_PATH)

file(COPY "${CMAKE_SOURCE_DIR}/../../../build/Debug/assets/shaders/examples/hello_triangle/hello_triangle.frag.spv"
     DESTINATION "${CMAKE_SOURCE_DIR}/app/src/main/assets/shaders/examples/hello_triangle/"
)
file(COPY "${CMAKE_SOURCE_DIR}/../../../build/Debug/assets/shaders/examples/hello_triangle/hello_triangle.vert.spv"
     DESTINATION "${CMAKE_SOURCE_DIR}/app/src/main/assets/shaders/examples/hello_triangle/"
)

set(APP_GLUE_DIR ${ANDROID_NDK_PATH}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library(
    app-glue STATIC
    ${APP_GLUE_DIR}/android_native_app_glue.c
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Declares and names the project.

project("HelloTriangle")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_FIND_DEBUG_MODE TRUE)

# Creates and names a library, sets it as either STATIC or SHARED, and provides
# the relative paths to its source code. You can define multiple libraries, and
# CMake builds them for you. Gradle automatically packages shared libraries with
# your APK.

add_library(
    # Sets the name of the library.
    hellotriangle
    # Sets the library as a shared library.
    SHARED
    # Provides a relative path to your source file(s).
    # SerenityNativeAndroid.cpp SerenityNativeAndroid.h
    hello_triangle.cpp main.cpp
)

target_include_directories(
    hellotriangle
    PRIVATE ../../../build/Debug/_deps/spdlog-src/include
            ../../../build/Debug/_deps/kdbindings-src/src
            ../../../build/Debug/_deps/kdutils-src/src
            ../../../build/Debug/_deps/kdutils-build/src
            ../../../build/Debug/_deps/vulkan_memory_allocator-src/include
            ../../../build/Debug/_deps/glm-src
            ../../../build/Debug/include
            ../../../src
)

add_library(
    KDFoundation STATIC IMPORTED
)
set_target_properties(
    KDFoundation PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDFoundation.a"
)
add_library(
    KDGui STATIC IMPORTED
)
set_target_properties(KDGui PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDGui.a")
add_library(
    KDUtils STATIC IMPORTED
)
set_target_properties(KDUtils PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDUtils.a")
add_library(
    spdlog STATIC IMPORTED
)
set_target_properties(spdlog PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libspdlogd.a")
add_library(
    KDGpu STATIC IMPORTED
)
set_target_properties(KDGpu PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDGpu.a")
add_library(
    KDGpuExample STATIC IMPORTED
)
set_target_properties(
    KDGpuExample PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDGpuExample.a"
)
add_library(
    imgui STATIC IMPORTED
)
set_target_properties(imgui PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libimgui.a")
add_library(
    KDGpuKDGui STATIC IMPORTED
)
set_target_properties(
    KDGpuKDGui PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDGpuKDGui.a"
)
add_library(
    KDGpuExampleResources STATIC IMPORTED
)
set_target_properties(
    KDGpuExampleResources PROPERTIES IMPORTED_LOCATION
                                     "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDGpuExampleResources.a"
)
add_library(
    KDGpuExampleShaderResources STATIC IMPORTED
)
set_target_properties(
    KDGpuExampleShaderResources
    PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../../../build/Debug/lib/libKDGpuExampleShaderResources.a"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DVK_USE_PLATFORM_ANDROID_KHR")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

target_link_libraries(
    # Specifies the target library.
    hellotriangle
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
    spdlog
)
