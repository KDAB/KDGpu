project(test-buffer VERSION 0.1 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    tst_buffer.cpp
)
target_link_libraries(${PROJECT_NAME} toy-renderer)

add_test(${PROJECT_NAME} ${PROJECT_NAME})
