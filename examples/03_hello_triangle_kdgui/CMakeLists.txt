project(03_kdgpu_hello_triangle_kdgui VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp hello_triangle.cpp
    LIBS KDGpu::KDGpu_KDGui)
