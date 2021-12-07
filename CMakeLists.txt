#!/bin/bash
cmake_minimum_required(VERSION 2.8)
project(bluesky)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -g -std=c++11 -Wall -pthread")
set(LIB_SRC 
    bluesky/log.cc
    bluesky/util.cc
    bluesky/config.cc
    )

find_library(YAMLCPP yaml-cpp)
message("***",${YAMLCPP})
#使用特定的源码为项目增加lib
add_library(bluesky SHARED ${LIB_SRC})



#指定生成目标
add_executable(test tests/test.cc)
add_dependencies(test bluesky)
#将目标文件与库文件进行链接
target_link_libraries(test bluesky ${YAMLCPP})

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config bluesky)
target_link_libraries(test_config bluesky ${YAMLCPP})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)