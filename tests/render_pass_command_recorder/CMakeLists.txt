project(test-render-pass-command-recorder VERSION 0.1 LANGUAGES CXX)

add_kdgpu_test(${PROJECT_NAME}
    tst_render_pass_command_recorder.cpp
)
