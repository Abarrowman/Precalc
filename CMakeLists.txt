cmake_minimum_required(VERSION 3.0.0)
project(Precalc)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

include_directories(src)
file(GLOB SOURCES src/*.cpp)

add_executable(Precalc ${SOURCES})