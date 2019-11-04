cmake_minimum_required(VERSION 3.9)
project(Wmderlandc VERSION 1.0.1)

# Find the required libraries.
find_package(X11 REQUIRED)

include_directories("src" "build")

# Grab all files end in .cc
add_executable(Wmderlandc wmderlandc.c)

set(LINK_LIBRARIES X11)
target_link_libraries(Wmderlandc ${LINK_LIBRARIES})

# Install rule
install(TARGETS Wmderlandc DESTINATION bin)
