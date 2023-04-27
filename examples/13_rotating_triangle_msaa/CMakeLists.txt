project(13_kdgpu_rotating_triangle_msaa VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp rotating_triangle_msaa.cpp
    LIBS KDGpu::KDGpuExample glm::glm
)
