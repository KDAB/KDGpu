project(
    05_kdgpu_rotating_triangle
    VERSION 0.1
    LANGUAGES CXX
)

find_package(Vulkan REQUIRED)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    rotating_triangle.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
)
