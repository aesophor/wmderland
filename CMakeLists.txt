cmake_minimum_required(VERSION 3.9)
project(wmderland VERSION 1.0.3)

# There are two CMake scripts in ./cmake
# 1. BuildType.cmake - determine build type on demand
# 2. Findglog.cmake - used by `find_package(glog REQUIRED)`
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(BuildType)

# Request C++14 standard, using new CMake variables.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -Wall")

# If the BuildType is Debug, then add -rdynamic.
# (used to print stacktrace with function names)
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
endif()


# Find the required libraries.
find_package(X11 REQUIRED)
find_package(glog)

# CMake will generate config.h from config.h.in
include_directories("src")
configure_file("src/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h")

# Grab all files end in .cc
FILE(GLOB cpp_sources src/*.cc)
add_executable(wmderland ${cpp_sources})

set(LINK_LIBRARIES X11)
if(GLOG_FOUND)
  set(LINK_LIBRARIES ${LINK_LIBRARIES} glog)
endif()
target_link_libraries(wmderland ${LINK_LIBRARIES})

# Install rule
install(TARGETS wmderland DESTINATION bin)
