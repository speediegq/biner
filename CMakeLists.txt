cmake_minimum_required(VERSION 3.1...3.29)

project(biner VERSION 0.1 LANGUAGES CXX)

set_property(GLOBAL PROPERTY CXX_STANDARD 2a)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    include
)

add_executable(biner
    src/biner.cpp
)

install(TARGETS biner DESTINATION bin)
