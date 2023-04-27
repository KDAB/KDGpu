project(kdgpu_multiview VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(NAME ${PROJECT_NAME}
    SOURCES main.cpp multiview.cpp
    LIBS KDGpu::KDGpuExample glm::glm)
