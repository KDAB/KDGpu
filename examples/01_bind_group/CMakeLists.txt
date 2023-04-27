project(01_kdgpu_bind_group VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp
    LIBS KDGpu::KDGpu KDGpu::KDGpuExample)
