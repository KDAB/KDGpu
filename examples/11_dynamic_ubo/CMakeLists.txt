project(11_kdgpu_dynamic_ubo VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp dynamic_ubo_triangles.cpp
    LIBS KDGpu::KDGpuExample glm::glm
)
