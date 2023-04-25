project(09_kdgpu_gradient_blobs VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp gradient_blobs.cpp
    LIBS KDGpu::KDGpu_KDGui glm::glm
)
