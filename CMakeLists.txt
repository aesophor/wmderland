cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -flto -Os -Wall -lpthread")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(Wmderland)
find_package(X11 REQUIRED)
find_package(glog REQUIRED)

# Grab all files end in .cpp
FILE(GLOB cpp_sources src/*.cpp)
add_executable(Wmderland ${cpp_sources})
target_link_libraries(Wmderland X11 glog)
