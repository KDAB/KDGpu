project(03_hello_triangle_serenity VERSION 0.1 LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    engine.cpp
    engine_layer.cpp
    example_engine_layer.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    toy-renderer-serenity
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_MACOS)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
