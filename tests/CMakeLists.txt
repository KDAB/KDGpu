cmake_minimum_required(VERSION 3.12)
project(Toy-Renderer-Tests)

include_directories(./doctest)

function(add_toy_renderer_test NAME SOURCES)
    add_executable(${NAME} ${SOURCES})

    target_link_libraries(${NAME} toy-renderer)

    add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
    set_tests_properties(${NAME} PROPERTIES LABELS "ToyRenderer")

    if(APPLE)
        target_compile_options(${NAME} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

function(add_toy_renderer_kdgui_test NAME SOURCES)
    add_executable(${NAME} ${SOURCES})

    target_link_libraries(${NAME} toy-renderer toy-renderer-kdgui)

    add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
    set_tests_properties(${NAME} PROPERTIES LABELS "ToyRendererKDGui")

    if(APPLE)
        target_compile_options(${NAME} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

add_subdirectory(pool)
add_subdirectory(buffer)
add_subdirectory(texture)
add_subdirectory(textureview)
add_subdirectory(instance)
add_subdirectory(bindgroup)
add_subdirectory(bindgrouplayout)
add_subdirectory(sampler)
add_subdirectory(compute_pipeline)
add_subdirectory(compute_pass_command_recorder)
add_subdirectory(command_recorder)
add_subdirectory(graphics_pipeline)
add_subdirectory(pipelinelayout)
