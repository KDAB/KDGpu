cmake_minimum_required(VERSION 3.12)
project(Toy-Renderer-Tests)

include_directories(./doctest)

add_subdirectory(pool)
add_subdirectory(buffer)
add_subdirectory(texture)
add_subdirectory(textureview)
