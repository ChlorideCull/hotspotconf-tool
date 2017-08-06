cmake_minimum_required(VERSION 3.8)
project(hotspotconf_tool)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

include_directories(include/)

set(SOURCE_FILES source/cdecode.c source/cencode.c source/main.c)

add_executable(hotspotconf_tool ${SOURCE_FILES})