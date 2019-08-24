#!/usr/bin/env bash

should_build=true
build_type=MINSIZEREL

function show_usage() {
  echo "Wmderland, A tiling window manager using space partitioning tree"
  echo "Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>"
  echo ""
  echo "<usage>: $0 [option]"
  echo "-b, --build   - Build project."
  echo "-p, --prepare - Prepare dev environment only, don't build."
}

if [ "$1" == "-p" ] || [ "$1" == "--prepare" ]; then
  should_build=false
elif [ "$1" == "-b" ] || [ "$1" == "--build" ] || [ -z "$1" ]; then
  should_build=true
elif [ "$1" == "-h" ] || [ "$1" == "--help" ] || [ "$1" == "-v" ] || [ "$1" == "--version" ] ; then
  show_usage
  exit 0
else
  show_usage
  exit 1
fi

# Build Wmderland
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=${build_type}
ln -sf ../build/config.h ../src/config.h
echo 'src/config.h symlink has been setup'

if [ $should_build == true ]; then
  make
fi

# Build Wmderlandc
cd ../ipc
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=${build_type}

if [ $should_build == true ]; then
  make
fi
