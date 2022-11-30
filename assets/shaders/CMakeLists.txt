project(shaders)

add_subdirectory(examples)

add_custom_target(toy_renderer_assets ALL)

foreach(shader ${SHADER_FILES})
    add_custom_command(TARGET toy_renderer_assets POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${shader} ${CMAKE_CURRENT_BINARY_DIR}/${shader})
endforeach(shader)