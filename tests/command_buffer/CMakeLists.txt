project(test-command-buffer VERSION 0.1 LANGUAGES CXX)

add_kdgpu_test(${PROJECT_NAME}
    tst_command_buffer.cpp
)
