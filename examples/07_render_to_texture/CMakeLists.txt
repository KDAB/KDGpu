project(07_kdgpu_render_to_texture VERSION 0.1 LANGUAGES CXX)

add_kdgpu_example(
    NAME ${PROJECT_NAME}
    SOURCES main.cpp render_to_texture.cpp
    LIBS KDGpu::KDGpu_KDGui glm::glm
)
