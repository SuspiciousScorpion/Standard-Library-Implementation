cmake_minimum_required(VERSION 3.20)
project(Heap_Test)

set(CMAKE_CXX_STANDARD 23)

add_executable(Heap_Test Heap_Test.cpp)
target_link_libraries(Heap_Test ${Boost_LIBRARIES})

