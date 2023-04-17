#TODO: we will probably want to have something more verbose in the future
add_custom_command(OUTPUT fonts
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} .
)

add_custom_target(font ALL
    DEPENDS fonts
)
