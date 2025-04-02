# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# Note: Starting with CMake version 3.21 FindVulkan.cmake will search for
# Vulkan_GLSLANG_VALIDATOR_EXECUTABLE by itself When requiring this version we
# can remove the next block then
if(NOT Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
    find_program(
        Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
        NAMES glslangValidator
        HINTS "$ENV{VULKAN_SDK}/bin"
    )
endif()
if(NOT Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
    message(FATAL_ERROR "glslangValidator executable not found")
endif()

# Compile a shader using glslangValidator
function(KDGpu_CompileShader target shader output)
    # If just the named args are present use them. If there is an optional 4th argument
    # we pass that in too. Useful for passing in -D define options to glslangValidator.
    if(${ARGC} EQUAL 3)
        add_custom_command(
            OUTPUT ${output}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
            COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} --quiet -V ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${output}
            COMMENT "Compile shader ${shader} using glslangValidator"
        )
    else()
        list(
            SUBLIST
            ARGV
            3
            -1
            REMAINING_ARGS
        )
        add_custom_command(
            OUTPUT ${output}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
            COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} --quiet -V ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${output}
                    ${REMAINING_ARGS}
            COMMENT "Compile shader ${shader} using glslangValidator"
        )
    endif()

    add_custom_target(
        ${target}
        DEPENDS ${output}
        COMMENT "Target to compile a shader"
    )
endfunction()

# Compile raytracing shader using glslangValidator
function(KDGpu_CompileRTShader target shader output)
    add_custom_command(
        OUTPUT ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} --quiet --target-env vulkan1.2
                ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${output}
        COMMENT "Compile shader ${shader} using glslangValidator"
    )

    add_custom_target(
        ${target}
        DEPENDS ${output}
        COMMENT "Target to compile a shader"
    )
endfunction()

# Compile task shader using glslangValidator
function(KDGpu_CompileMeshTaskShader target shader output)
    add_custom_command(
        OUTPUT ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} --quiet --target-env vulkan1.2
                ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${output}
        COMMENT "Compile Mesh/Task shader ${shader} using glslangValidator"
    )

    add_custom_target(
        ${target}
        DEPENDS ${output}
        COMMENT "Target to compile a mesh/task shader"
    )
endfunction()

# Compile a shader set with a vertex and fragment shaders
function(KDGpu_CompileShaderSet target name)
    # TODO: in future we probably want to check which shaders we have instead of assuming vert/frag
    kdgpu_compileshader(${target}VertexShader ${name}.vert ${name}.vert.spv)
    kdgpu_compileshader(${target}FragmentShader ${name}.frag ${name}.frag.spv)

    # TODO: for now generate ALL, in future would be better to build on case by case
    add_custom_target(
        ${target}Shaders ALL
        DEPENDS ${target}VertexShader ${target}FragmentShader
        COMMENT "Target to compile a shader set"
    )
endfunction()

# Compile a shader set with a vertex, geometry and fragment shaders
function(KDGpu_CompileGeomShaderSet target name)
    kdgpu_compileshader(${target}VertexShader ${name}.vert ${name}.vert.spv)
    kdgpu_compileshader(${target}GeometryShader ${name}.geom ${name}.geom.spv)
    kdgpu_compileshader(${target}FragmentShader ${name}.frag ${name}.frag.spv)

    add_custom_target(
        ${target}Shaders ALL
        DEPENDS ${target}VertexShader ${target}GeometryShader ${target}FragmentShader
        COMMENT "Target to compile a shader set with a geometry shader"
    )
endfunction()

# Compile a shader using dxc
function(
    KDGpu_CompileHLSLShader
    target
    shader
    output
    type
)
    add_custom_command(
        OUTPUT ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMAND ${DXC_EXECUTABLE} -Emain -T${type}_6_1 -Zi $<IF:$<CONFIG:DEBUG>,-Od,-O3> -spirv -Fo${output}
                ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMENT "Compile shader ${shader} using dxc"
    )

    add_custom_target(
        ${target}
        DEPENDS ${output}
        COMMENT "Target to compile a shader"
    )
endfunction()

# Compiles shader set using dxc
function(KDGpu_CompileHLSLShaderSet target name)
    # TODO: in future we probably want to check which shaders we have instead of assuming vert/frag
    kdgpu_compilehlslshader(${target}VertexShader ${name}.ps.hlsl ${name}.ps.spv ps)
    kdgpu_compilehlslshader(${target}FragmentShader ${name}.vs.hlsl ${name}.vs.spv vs)

    # TODO: for now generate ALL, in future would be better to build on case by case
    add_custom_target(
        ${target}Shaders ALL
        DEPENDS ${target}VertexShader ${target}FragmentShader
        COMMENT "Target to compile a shader set using dxc"
    )
endfunction()
