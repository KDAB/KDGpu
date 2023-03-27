project(test-instance VERSION 0.1 LANGUAGES CXX)

add_kdgpu_kdgui_test(${PROJECT_NAME}
    tst_instance.cpp
)
