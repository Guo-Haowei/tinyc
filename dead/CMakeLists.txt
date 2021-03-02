cmake_minimum_required(VERSION 3.1)
project(toy_c C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(code)
add_subdirectory(test)
