cmake_minimum_required(VERSION 3.10)
project(model-load VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

find_package(assimp REQUIRED)
if(NOT assimp_FOUND)
    message(STATUS ">>> assimp not found")
endif(NOT assimp_FOUND)

set(THIRD_PARTY_DIR "third_party/")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${THIRD_PARTY_DIR})

if(assimp_FOUND)
    set(EXTRA_INCLUDE_DIR ${assimp_INCLUDE_DIRS})
    target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${EXTRA_INCLUDE_DIR})
    target_link_libraries(${CMAKE_PROJECT_NAME} ${assimp_LIBRARIES})
endif()
