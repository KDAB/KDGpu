cmake_minimum_required(VERSION 3.12)
project(KDGpu-Tests)

include_directories(./doctest)

function(add_kdgpu_test NAME SOURCES)
    set(TARGET_NAME KDGpu_${NAME})
    add_executable(${TARGET_NAME} ${SOURCES})

    target_link_libraries(${TARGET_NAME} KDGpu)

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpu")

    if(APPLE)
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

function(add_kdgpu_kdgui_test NAME SOURCES)
    set(TARGET_NAME KDGpu_KDGui_${NAME})
    add_executable(${TARGET_NAME} ${SOURCES})

    target_link_libraries(${TARGET_NAME} KDGpu KDGpu_KDGui)

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "KDGpu_KDGui")

    if(APPLE)
        target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
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
add_subdirectory(command_buffer)
add_subdirectory(graphics_pipeline)
add_subdirectory(pipelinelayout)
add_subdirectory(fence)
