function(add_kdgpu_example)
    include(CMakeParseArguments)
    cmake_parse_arguments(
        KD_ARGS
        ""
        "NAME"
        "SOURCES;LIBS"
        ${ARGN}
    )

    add_executable(${KD_ARGS_NAME} ${KD_ARGS_SOURCES})

    if(WIN32)
        # force no console window, but keep main() as cross-platform entry point
        target_link_options(${KD_ARGS_NAME} PRIVATE "/SUBSYSTEM:windows" "/ENTRY:mainCRTStartup")
    endif()

    target_link_libraries(${KD_ARGS_NAME} PRIVATE ${KD_ARGS_LIBS})

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE PLATFORM_LINUX)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE PLATFORM_WIN32)
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE UNICODE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_compile_definitions(${KD_ARGS_NAME} PRIVATE PLATFORM_MACOS)
    endif()

    target_compile_features(${KD_ARGS_NAME} PUBLIC cxx_std_20)

    # set rpath for the installed binary to work
    if(APPLE)
        set_target_properties(${KD_ARGS_NAME} PROPERTIES INSTALL_RPATH "@loader_path;${CMAKE_INSTALL_FULL_LIBDIR}")
    elseif(UNIX)
        set_target_properties(${KD_ARGS_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/:${CMAKE_INSTALL_FULL_LIBDIR}")
    endif()

    install(TARGETS ${KD_ARGS_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction()

add_subdirectory(01_bind_group)
add_subdirectory(02_hello_triangle)
add_subdirectory(03_hello_triangle_kdgui)
add_subdirectory(04_hello_indexed_triangle)
add_subdirectory(05_rotating_triangle)
add_subdirectory(06_compute_particles)
add_subdirectory(07_render_to_texture)
add_subdirectory(08_hello_triangle_kdgui_fence)
add_subdirectory(09_gradient_blobs)
add_subdirectory(10_styled_rectangles)
add_subdirectory(11_dynamic_ubo)
add_subdirectory(12_textured_quad)
add_subdirectory(13_rotating_triangle_msaa)
add_subdirectory(offscreen_rendering)
