cmake_minimum_required(VERSION 3.10)
project(model-load VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_package(assimp REQUIRED)
if(NOT ASSIMP_FOUND)
    message(STATUS ">>> assimp not found")
endif(NOT ASSIMP_FOUND)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

