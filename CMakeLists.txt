cmake_minimum_required(VERSION 3.28)
project(o2c++)

set(CMAKE_CXX_STANDARD 17)

add_executable(o2c++ main.cpp Token.cpp Scanner.cpp)

add_executable(Hello Hello-main.cpp Hello.cpp Out.cpp)
