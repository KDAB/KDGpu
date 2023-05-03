project(
    test-bindgroup
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_test(${PROJECT_NAME} tst_bindgroup.cpp)
