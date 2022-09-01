cmake_minimum_required(VERSION 3.9)
project(wmderlandc VERSION 1.0.5)

find_package(X11 REQUIRED)
include_directories("src" "build")
add_executable(wmderlandc wmderlandc.c)

set(LINK_LIBRARIES X11)
target_link_libraries(wmderlandc ${LINK_LIBRARIES})

install(TARGETS wmderlandc DESTINATION bin)
