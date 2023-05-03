project(
    10_kdgpu_styled_rectangles
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    styled_rectangles.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
)
