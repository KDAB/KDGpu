project(shaders)

add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(kdgpu_kdgui)

add_custom_target(kdgpu_assets ALL)

foreach(shader ${SHADER_FILES})
    add_custom_command(
        TARGET kdgpu_assets
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${shader} ${CMAKE_CURRENT_BINARY_DIR}/${shader}
    )
endforeach(shader)
