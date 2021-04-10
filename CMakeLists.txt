cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(SET CMP0091 NEW) ## PROPERTY MSVC_RUNTIME_LIBRARY 

set(PROJECT BuildInc)

enable_language(C)
project(${PROJECT} C)
add_executable(${PROJECT} BuildInc.c Build.h)
install(TARGETS ${PROJECT} DESTINATION "${CMAKE_SOURCE_DIR}/include/BuildInc")
