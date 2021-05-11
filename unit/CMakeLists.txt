cmake_minimum_required(VERSION 2.6)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

#Locate GTest
find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test.out
    ../list.c
    ../string_util.c
    list.test.cpp
    string_util.test.cpp
    test.cpp
)

target_compile_definitions(test.out PRIVATE -DTARGET_BUILD=2)
target_link_libraries(test.out ${GTEST_LIBRARIES} pthread)