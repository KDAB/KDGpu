# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
project(
    kdgpu_hello_triangle_apple
    VERSION 0.1
    LANGUAGES CXX
)

# generate spirv shaders
function(add_shaders)
    include(CMakeParseArguments)
    cmake_parse_arguments(
        PA
        "FLAT"
        "NAME"
        ""
        ${ARGN}
    )
    if(PA_FLAT)
        set(RESOURCE_DIR ${CMAKE_BINARY_DIR}/bin/Debug/${ARGV0}.app/)
    else()
        set(RESOURCE_DIR ${CMAKE_BINARY_DIR}/bin/${ARGV0}.app/Contents/Resources)
    endif()
    CompileShader(${ARGV0}VertexShader ${PA_NAME}.vert ${RESOURCE_DIR}/${PA_NAME}.vert.spv)
    CompileShader(${ARGV0}FragmentShader ${PA_NAME}.frag ${RESOURCE_DIR}/${PA_NAME}.frag.spv)

    add_custom_target(
        ${ARGV0}Shaders ALL
        DEPENDS ${ARGV0}VertexShader ${ARGV0}FragmentShader
        COMMENT "Target to compile a shader set"
    )
endfunction()

find_program(IBTOOL_EXECUTABLE ibtool)

# Compile storyboards
function(add_storyboard)
    include(CMakeParseArguments)
    cmake_parse_arguments(
        PA
        "FLAT"
        "STORYBOARD_DIR"
        ""
        ${ARGN}
    )

    set(STORYBOARD_FILE Main.storyboard)
    if(PA_FLAT)
        set(RESOURCE_DIR ${CMAKE_BINARY_DIR}/bin/Debug/${ARGV0}.app/)
    else()
        set(RESOURCE_DIR ${CMAKE_BINARY_DIR}/bin/${ARGV0}.app/Contents/Resources)
    endif()
    add_custom_command(
        OUTPUT ${RESOURCE_DIR}/${STORYBOARD_FILE}c
        COMMAND ${IBTOOL_EXECUTABLE} --compile ${RESOURCE_DIR}/${STORYBOARD_FILE}c
                ${PA_STORYBOARD_DIR}/${STORYBOARD_FILE}
        DEPENDS ${PA_STORYBOARD_DIR}/${STORYBOARD_FILE}
        COMMENT "Compiling storyboards"
    )
    target_sources(${ARGV0} PRIVATE ${RESOURCE_DIR}/${STORYBOARD_FILE}c ${PA_STORYBOARD_DIR}/${STORYBOARD_FILE})
endfunction()

# Add an example for Apple (macOS, iOS, etc) platforms
function(add_apple_example)
    include(CMakeParseArguments)
    cmake_parse_arguments(
        PA
        ""
        "PLIST"
        "SOURCES;LIBS;INCLUDE;DEFINES"
        ${ARGN}
    )

    target_sources(${ARGV0} PRIVATE ${PA_SOURCES})
    target_include_directories(${ARGV0} PRIVATE ${PA_INCLUDE})
    target_compile_definitions(${ARGV0} PRIVATE ${PA_DEFINES})
    target_compile_options(${ARGV0} PRIVATE -fmodules -fobjc-arc)
    target_link_libraries(${ARGV0} PUBLIC KDGpu::KDGpu ${PA_LIBS})
    set_target_properties(
        ${ARGV0}
        PROPERTIES BUNDLE True
                   MACOSX_BUNDLE_GUI_IDENTIFIER com.example.KDGpuHelloTriangleApple
                   MACOSX_BUNDLE_BUNDLE_NAME kdgpu_hello_triangle_apple
                   MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
                   MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
                   MACOSX_BUNDLE_INFO_PLIST ${PA_PLIST}
    )
endfunction()

add_executable(
    kdgpu_hello_triangle_apple MACOSX_BUNDLE
    Application/AAPLAppDelegate.h
    Application/AAPLAppDelegate.m
    Application/AAPLConfig.h
    Application/AAPLView.h
    Application/AAPLView.m
    Application/AAPLViewController.h
    Application/AAPLViewController.m
    Application/main.m
    Renderer/AAPLRenderer.h
    Renderer/AAPLRenderer.mm
    Renderer/KDGpuRenderer.cpp
    Renderer/KDGpuRenderer.h
)

target_include_directories(
    kdgpu_hello_triangle_apple
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Application ${CMAKE_CURRENT_SOURCE_DIR}/Renderer
)

add_dependencies(kdgpu_hello_triangle_apple kdgpu_hello_triangle_appleShaders)

if(CMAKE_SYSTEM_NAME MATCHES "iOS")
    add_shaders(kdgpu_hello_triangle_apple NAME hello_triangle FLAT)
    add_apple_example(
        kdgpu_hello_triangle_apple
        SOURCES
        Application/UIKit/AAPLUIView.h
        Application/UIKit/AAPLUIView.m
        INCLUDE
        ${CMAKE_CURRENT_SOURCE_DIR}/Application/UIKit
        DEFINES
        TARGET_IOS
        LIBS
        "-framework QuartzCore"
        "-framework UIKit"
        PLIST
        ${CMAKE_CURRENT_SOURCE_DIR}/Application/UIKit/iOS/Info.plist.in
    )
    add_storyboard(
        kdgpu_hello_triangle_apple STORYBOARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Application/UIKit/iOS/Base.lproj FLAT
    )
else()
    add_shaders(kdgpu_hello_triangle_apple NAME hello_triangle)
    add_apple_example(
        kdgpu_hello_triangle_apple
        SOURCES
        Application/AppKit/AAPLNSView.h
        Application/AppKit/AAPLNSView.m
        INCLUDE
        ${CMAKE_CURRENT_SOURCE_DIR}/Application/AppKit
        DEFINES
        TARGET_MACOS
        LIBS
        "-framework Cocoa"
        "-framework QuartzCore"
        "-framework AppKit"
        PLIST
        ${CMAKE_CURRENT_SOURCE_DIR}/Application/AppKit/macOS/Info.plist.in
    )
    add_storyboard(
        kdgpu_hello_triangle_apple STORYBOARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Application/AppKit/macOS/Base.lproj
    )
endif()
