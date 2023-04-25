project(04_kdgpu_hello_indexed_triangle VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp hello_indexed_triangle.cpp
    LIBS KDGpu::KDGpu_KDGui)
