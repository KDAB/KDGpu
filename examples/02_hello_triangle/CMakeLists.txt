project(02_kdgpu_hello_triangle VERSION 0.1 LANGUAGES CXX)

find_package(Vulkan REQUIRED)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp
    LIBS KDGpu KDGpu_KDGui glm::glm)
