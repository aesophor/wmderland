<div align="center">
<h3>Wmderland</h3>
Made during <a href="https://www.facebook.com/events/256671588330840/">NTUOSC Hackathon 2018</a>
<img src="https://github.com/aesophor/Wmderland/raw/master/assets/tiling.png">
</div>

## Overview
Heavily Inspired by [i3wm](https://github.com/i3/i3). Wmderland aims to simplify the core functionalities of i3wm, and bundle in the essential features required for a modern but minimal Tiling Window Manager.

Written in C++ using [Xlib](https://en.wikipedia.org/wiki/Xlib)

## Why Another WM
I started this project because I want experience with OOP in C++ and Xlib. I know there's already [lots of WM out there](https://wiki.archlinux.org/index.php/Window_manager), but I'm writing this for my personal use and study.

## Build Requirements
* g++ (requires c++11)
* cmake
* Xlib headers
* [glog](https://github.com/google/glog) (Google's c++ logging library)

## Installation
1. Build and install from source (make sure you have **GLOG** installed, see Build Requirements)
```
$ git clone https://github.com/aesophor/Wmderland
$ cd Wmderland
```

2. Run CMake and copy the executable to /usr/local/bin
```
$ mkdir build
$ cmake .. -DCMAKE_BUILD_TYPE=MINSIZEREL
$ make
$ sudo cp Wmderland /usr/local/bin
```

3. Copy the config file (**IMPORTANT**)
```
$ mkdir -p ~/.config/Wmderland
$ cp example/config ~/.config/Wmderland/.
```

4. Add this line to your ~/.xinitrc
```
# Fix Non-reparenting window managers / Grey window /
# Programs not drawing properly
export _JAVA_AWT_WM_NONREPARENTING=1

# Startup Wmderland
exec Wmderland
```

5. Initialize an X session
```
$ startx
```

## Features
* Tree-based horizontal and vertical tiling.
* Gaps and borders.
* Toggle windows between tiled / floating.
* Smart floating (dialog windows will be floating by default).
* Workspaces. ( `Mod4 + {1~9}`: switch; `Mod4 + Shift + {1~9}`: move application to... )
* Configuration supported. ([example config](https://github.com/aesophor/Wmderland/blob/master/example/config))
* Remembers the position and size of floating windows.
* Supports a subset of EWMH, please see `src/properties.cpp`.
* Compatible with Polybar's `xwindow` and `xworkspaces` module.
* Wine applications will not hang on close (fixed in commit [`a816f31`](https://github.com/aesophor/Wmderland/commit/a816f312d4f6b06865d36bbb565be95475d71719#comments))

## Todo List
* \_NET_WM_WINDOW_TYPE_SPLASH should be floating by default
* WINE Steam is extremely buggy
* Two borders
* Improve stability

## Other Screenshots
Floating windows
![](https://github.com/aesophor/Wmderland/raw/master/assets/floating.png)

## Special Thanks
* suckless.org's dwm
* jichu4n's [basic_wm](https://github.com/jichu4n/basic_wm)
* mil's [simple_wm](https://github.com/mil/simple-wm)
* JLErvin's [berry](https://github.com/JLErvin/berry)
* Mr messyhair's [eveningwm](https://gitlab.com/mrmessyhair/eveningwm/blob/master/eveningwm.c)

## License
Available under the [MIT License](https://github.com/aesophor/Wmderland/blob/master/LICENSE)
