project(test-compute-pipeline VERSION 0.1 LANGUAGES CXX)

add_kdgpu_test(${PROJECT_NAME}
    tst_compute_pipeline.cpp
)
