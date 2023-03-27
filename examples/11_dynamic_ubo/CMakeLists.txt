project(11_kdgpu_dynamic_ubo VERSION 0.1 LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    dynamic_ubo_triangles.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    KDGpu::KDGpu_KDGui
    glm::glm
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
