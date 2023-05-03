project(
    kdgpu_offscreen_rendering
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    offscreen.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
    stb::stb
)
