cmake_minimum_required(VERSION 3.0)

# set (CMAKE_C_COMPILER "/usr/local/gcc/bin/gcc")
# set (CMAKE_CXX_COMPILER "/usr/local/gcc/bin/g++")

project(ChatRoom)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE ALL_RESOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE ALL_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(server ${ALL_RESOURCE})
#add_executable(client ${ALL_RESOURCE} client.cpp)
