find_program(GLSLANG_VALIDATOR glslangValidator)

function(CompileShader target shader output)
    add_custom_command(OUTPUT ${output}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${output}
    )

    add_custom_target(${target}
        DEPENDS ${output}
    )
endfunction()

function(CompileShaderSet target name)
    # TODO: in future we probably want to check which shaders we have instead of assuming vert/frag
    CompileShader(${target}VertexShader ${name}.vert ${name}.vert.spv)
    CompileShader(${target}FragmentShader ${name}.frag ${name}.frag.spv)

    # TODO: for now generate ALL, in future would be better to build on case by case
    add_custom_target(${target}Shaders ALL
        DEPENDS ${target}VertexShader
        DEPENDS ${target}FragmentShader
    )
endfunction()
