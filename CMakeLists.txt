cmake_minimum_required(VERSION 3.9)
project(Wmderland)

# Request C++11 standard, using new CMake variables.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -Wall")

# There are two CMake scripts in ./cmake
# 1. BuildType.cmake - determine build type on demand
# 2. Findglog.cmake - used by `find_package(glog REQUIRED)`
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Determine BuildType on demand.
include(BuildType)

# Find the required libraries.
find_package(X11 REQUIRED)
find_package(glog REQUIRED)

# CMake will generate config.hpp from config.hpp.in
# and store it under src/
configure_file(
    "${PROJECT_SOURCE_DIR}/src/config.hpp.in"
    "${PROJECT_SOURCE_DIR}/src/config.hpp"
)

# Grab all files end in .cpp
FILE(GLOB cpp_sources src/*.cpp)
add_executable(Wmderland ${cpp_sources})

set(LIBRARIES "X11")
if(GLOG_FOUND)
    set(LIBRARIES "${LIBRARIES} glog")
endif()
target_link_libraries(Wmderland X11 ${LIBRARIES})

# Install rule
install(TARGETS Wmderland DESTINATION bin)
