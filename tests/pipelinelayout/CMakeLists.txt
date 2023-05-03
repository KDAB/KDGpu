project(
    test-pipeline-layout
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_test(${PROJECT_NAME} tst_pipeline_layout.cpp)
