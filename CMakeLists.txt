cmake_minimum_required(VERSION 3.1)

set(PROJECT BuildInc)
enable_language(CXX)
project(${PROJECT} CXX)

set(EZLIBS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ezlibs/include)
include_directories(${EZLIBS_INCLUDE_DIR})

file(GLOB_RECURSE EZ_LIBS_SOURCES ${EZLIBS_INCLUDE_DIR}/ezlibs/*.hpp)
source_group(TREE ${EZLIBS_INCLUDE_DIR}/ezlibs PREFIX ezlibs FILES ${EZ_LIBS_SOURCES})

add_executable(${PROJECT} main.cpp ${EZ_LIBS_SOURCES})

set_target_properties(${PROJECT} PROPERTIES FOLDER tools)
