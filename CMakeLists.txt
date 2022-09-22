# This repo is handled by jetbrains fleet
project(comphsics-adt CXX)
cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(test)
