cmake_minimum_required(VERSION 3.21)
project(test_package CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(ballistic-engine REQUIRED)

add_executable(test_package main.cpp)
target_link_libraries(test_package PRIVATE ballistic::engine)
