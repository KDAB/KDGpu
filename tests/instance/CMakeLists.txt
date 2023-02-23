project(test-instance VERSION 0.1 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    tst_instance.cpp
)
target_link_libraries(${PROJECT_NAME} toy-renderer toy-renderer-kdgui)

add_test(${PROJECT_NAME} ${PROJECT_NAME})
