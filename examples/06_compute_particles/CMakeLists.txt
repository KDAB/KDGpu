project(
    06_kdgpu_compute_particles
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_example(
    NAME
    ${PROJECT_NAME}
    SOURCES
    main.cpp
    compute_particles.cpp
    LIBS
    KDGpu::KDGpuExample
    glm::glm
)
