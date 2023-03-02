project(test-command-recorder VERSION 0.1 LANGUAGES CXX)

add_toy_renderer_test(${PROJECT_NAME}
    tst_command_recorder.cpp
)
