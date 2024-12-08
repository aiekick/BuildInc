cmake_minimum_required(VERSION 3.1)

set(PROJECT BuildInc)
enable_language(CXX)
project(${PROJECT} CXX)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/ezlibs/include)

add_executable(${PROJECT} main.cpp)

set_target_properties(${PROJECT} PROPERTIES FOLDER tools)
