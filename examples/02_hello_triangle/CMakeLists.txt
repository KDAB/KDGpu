project(02_kdgpu_hello_triangle VERSION 0.1 LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_executable(
    ${PROJECT_NAME}
    main.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    KDGpu::KDGpu
    KDUtils::Gui
    glm::glm
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_MACOS)
    target_sources(${PROJECT_NAME} PRIVATE utils.mm)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
