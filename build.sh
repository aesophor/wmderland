#!/usr/bin/env bash

should_install=false
build_failed=false
build_type=MINSIZEREL


function show_usage() {
  echo "Wmderland, A tiling window manager using space partitioning tree"
  echo "Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>"
  echo ""
  echo "usage: $0 [option]"
  echo "-i, --install - Build and install project (sudo make install)"
  echo "-h, --help    - Show this help message"
}

function show_horizontal_line() {
  printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' -
}

# Build main project
function build_wmderland() {
  echo "-- Building Wmderland (WM)"
  mkdir -p build && cd build

  cmake .. -DCMAKE_BUILD_TYPE=${build_type}
  make

  if [ $? != 0 ]; then
    build_failed=true
  fi

  if [ $should_install == true ]; then
    echo ""
    echo "-- Installing Wmderland (WM), invoked with sudo make install"
    sudo make install && echo -e "-- Installed to "`cat install_manifest.txt`"\n"
    sudo cp ../example/Wmderland.desktop /usr/share/xsessions/.
  fi

  cd ..
}

# Build client
function build_client() {
  show_horizontal_line
  echo "-- Building Wmderlandc (client)"
  cd ipc-client
  mkdir -p build && cd build

  cmake .. -DCMAKE_BUILD_TYPE=${build_type}
  make

  if [ $? != 0 ]; then
    build_failed=true
  fi

  if [ $should_install == true ]; then
    echo ""
    echo "-- Installing Wmderlandc (client), invoked with sudo make install"
    sudo make install && echo -e "-- Installed to "`cat install_manifest.txt`"\n"
  fi

  cd ../..
}

function build() {
  build_wmderland
  build_client

  if [ $build_failed == true ]; then
    return 1;
  fi
}

# $1 - args array
# $2 - the target argument to match
function has_argument() {
  args=("$@")
  target=${args[${#args[@]}-1]} # extract target argument
  unset 'args[${#args[@]}-1]' # remove last element

  for arg in "${args[@]}"; do
    if [ "$arg" == "$target" ]; then
      return 0
    fi
  done
  return 1
}


(has_argument $@ '-h' || has_argument $@ '--help') && show_usage && exit 0
(has_argument $@ '-i' || has_argument $@ '--install') && should_install=true
build
