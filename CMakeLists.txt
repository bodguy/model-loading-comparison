cmake_minimum_required(VERSION 3.10)
project(model-load VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
set(EXTRA_INCLUDE_DIR "third_party/")
set(EXTRA_LIBRARIES "")

find_package(assimp REQUIRED)
if(assimp_FOUND)
    list(APPEND EXTRA_INCLUDE_DIR "${assimp_INCLUDE_DIRS}")
    list(APPEND EXTRA_LIBRARIES "${assimp_LIBRARIES}")
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${EXTRA_INCLUDE_DIR})
target_link_libraries(${CMAKE_PROJECT_NAME} ${EXTRA_LIBRARIES})
