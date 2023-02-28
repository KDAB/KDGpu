project(test-bindgrouplayout VERSION 0.1 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    tst_bindgroup_layout.cpp
)
target_link_libraries(${PROJECT_NAME} toy-renderer)

add_test(${PROJECT_NAME} ${PROJECT_NAME})
