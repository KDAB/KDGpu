project(
    test-graphics-pipeline
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_test(${PROJECT_NAME} tst_graphics_pipeline.cpp)
