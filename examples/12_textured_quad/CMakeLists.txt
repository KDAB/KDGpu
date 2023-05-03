project(
    12_kdgpu_textured_quad
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    textured_quad.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
    stb::stb
)
