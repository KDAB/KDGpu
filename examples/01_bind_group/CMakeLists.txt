project(01_kdgpu_bind_group VERSION 0.1 LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} KDGpu KDGpu_KDGui)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
