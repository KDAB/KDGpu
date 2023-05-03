find_program(GLSLANG_VALIDATOR glslangValidator)

# Compile a shader using glslangValidator
function(CompileShader target shader output)
    add_custom_command(
        OUTPUT ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${output}
        COMMENT "Compile shader using glslangValidator"
    )

    add_custom_target(
        ${target}
        DEPENDS ${output}
        COMMENT "Target to compile a shader"
    )
endfunction()

# Compile s shader set
function(CompileShaderSet target name)
    # TODO: in future we probably want to check which shaders we have instead of assuming vert/frag
    CompileShader(${target}VertexShader ${name}.vert ${name}.vert.spv)
    CompileShader(${target}FragmentShader ${name}.frag ${name}.frag.spv)

    # TODO: for now generate ALL, in future would be better to build on case by case
    add_custom_target(
        ${target}Shaders ALL
        DEPENDS ${target}VertexShader ${target}FragmentShader
        COMMENT "Target to compile a shader set"
    )
endfunction()
