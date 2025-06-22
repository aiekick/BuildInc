cmake_minimum_required(VERSION 3.20)

set(PROJECT BuildInc)

project(${PROJECT} CXX)
enable_language(CXX)

set(EZLIBS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
include_directories(${EZLIBS_INCLUDE_DIR})

add_executable(${PROJECT} main.cpp)

set_target_properties(${PROJECT} PROPERTIES FOLDER tools)
