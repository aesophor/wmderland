cmake_minimum_required(VERSION 3.9)
project(wmderland VERSION 1.0.5)

# There are two CMake scripts in ./cmake
# 1. BuildType.cmake - determine build type on demand
# 2. Findglog.cmake - used by `find_package(glog REQUIRED)`
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(BuildType)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -Wall")

# If the BuildType is Debug, then add -rdynamic.
# (used to print stacktrace with function names)
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
endif()

find_package(X11 REQUIRED)
find_package(glog)

# CMake will generate config.h from config.h.in
include_directories("src")
configure_file("src/config.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h")

add_executable(
  ${PROJECT_NAME}

  src/action.cc
  src/client.cc
  src/config.cc
  src/cookie.cc
  src/ipc.cc
  src/main.cc
  src/mouse.cc
  src/properties.cc
  src/snapshot.cc
  src/stacktrace.cc
  src/tree.cc
  src/util.cc
  src/window_manager.cc
  src/workspace.cc
)

set(LINK_LIBRARIES X11)
if (GLOG_FOUND)
  set(LINK_LIBRARIES ${LINK_LIBRARIES} glog)
endif()
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
