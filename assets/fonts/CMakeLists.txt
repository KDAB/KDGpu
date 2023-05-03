# TODO: we will probably want to have something more verbose in the future
add_custom_command(
    OUTPUT KDGpuKDGui_fonts
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} .
    COMMENT "Copy font assets to the current source directory"
)

add_custom_target(
    KDGpuKDGui_font ALL
    DEPENDS KDGpuKDGui_fonts
    COMMENT "Target to add the font assets"
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/Roboto-Medium.ttf ${CMAKE_CURRENT_SOURCE_DIR}/Roboto-Medium-license.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/assets/fonts
)
