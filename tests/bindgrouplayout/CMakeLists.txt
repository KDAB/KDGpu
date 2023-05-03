project(
    test-bindgrouplayout
    VERSION 0.1
    LANGUAGES CXX
)

add_kdgpu_test(${PROJECT_NAME} tst_bindgroup_layout.cpp)
