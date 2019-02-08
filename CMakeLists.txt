cmake_minimum_required(VERSION 3.9)
project(Wmderland)

# Request C++11 standard, using new CMake variables.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# There are two CMake scripts in ./cmake
# 1. BuildType.cmake - determine build type on demand
# 2. Findglog.cmake - used by `find_package(glog REQUIRED)`
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Load and run CMake code from a file or module.
include(BuildType)

# Add some extra flags to g++
# -flto: runs the standard link-time optimizer
# -Wall: "warn all" turns on almost all warning
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -Wall")

find_package(X11 REQUIRED)
find_package(glog REQUIRED)

# Grab all files end in .cpp
FILE(GLOB cpp_sources src/*.cpp)
add_executable(Wmderland ${cpp_sources})
target_link_libraries(Wmderland X11 glog)
