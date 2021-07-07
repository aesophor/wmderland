cmake_minimum_required(VERSION 3.9)
project(wmderlandc VERSION 1.0.4)

# Find the required libraries.
find_package(X11 REQUIRED)

if(APPLE)
  include_directories("src" "build" "/opt/X11/include")
  link_directories("/opt/X11/lib")
else()
  include_directories("src" "build")
endif()

# Grab all files end in .cc
add_executable(wmderlandc wmderlandc.c)

set(LINK_LIBRARIES X11)
target_link_libraries(wmderlandc ${LINK_LIBRARIES})

# Install rule
install(TARGETS wmderlandc DESTINATION bin)
