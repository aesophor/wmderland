<div align="center">
<h3>Wmderland</h3>
Project started in <a href="https://www.facebook.com/events/256671588330840/">NTUOSC Hackathon 2018</a>
<img src="https://github.com/aesophor/Wmderland/raw/master/assets/tiling.png">
</div>

## Overview
Wmderland is an X11 Dynamic Window Manager that represents the windows as the leaves of a tree.

It aims to simplify the core functionalities of [i3wm](https://github.com/i3/i3), which features a more approachable config system, and bundle in the essential features required for a modern but minimal Dynamic Window Manager.

Written in C++ using [Xlib](https://en.wikipedia.org/wiki/Xlib)

## Why Another WM
I started this project because I want experience with OOP in C++ and Xlib. I know there's already [lots of WM out there](https://wiki.archlinux.org/index.php/Window_manager), but I'm writing this for my personal use and study.

## Internal Data Structures
Please note that **"Clients"** are the windows that are managed by our WM.

In Wmderland, clients are represented as the leaves of a tree, while the internal nodes tell the split direction of its children. The root node always exists with the client tree (it WON'T and MUSTN'T be deleted). The clients are arranged (tiled) recursively.

The following illustration shows how Wmderland stores its clients, where R is the root node, V means its children must be tiled vertically, H means its children must be tiled horizontally.

```
             R                         R                          R
                                      /                          / \
                        --->         1             --->         1   2
                                     ^                              ^

+-----------------------+  +-----------------------+  +-----------------------+
|                       |  |                       |  |           |           |
|                       |  |                       |  |           |           |
|           R           |  |           1           |  |     1     |     2     |
|                       |  |           ^           |  |           |     ^     |
|                       |  |                       |  |           |           |
|                       |  |                       |  |           |           |
|                       |  |                       |  |           |           |
+-----------------------+  +-----------------------+  +-----------------------+

          Empty                      Spawn 1                   Spawn 2 

```

```
            R                          R                           R
           /|\                        /|\                         /|\
          1 2 3         --->         1 2 V         --->          1 2 V
              ^                         / \                         / \
                                       3   4                       3   H
                                           ^                          / \
                                                                     4   5
                                                                         ^

+-----------------------+  +-----------------------+  +-----------------------+
|       |       |       |  |       |       |       |  |       |       |       |
|       |       |       |  |       |       |   3   |  |       |       |   3   |
|   1   |   2   |   3   |  |   1   |   2   |       |  |   1   |   2   |       |
|       |       |   ^   |  |       |       |-------|  |       |       |-------|
|       |       |       |  |       |       |       |  |       |       |   |   |
|       |       |       |  |       |       |   4   |  |       |       | 4 | 5 |
|       |       |       |  |       |       |   ^   |  |       |       |   | ^ |
+-----------------------+  +-----------------------+  +-----------------------+

         Spawn 3               $Mod+V and Spawn 4        $Mod+G and Spawn 5
         
```

* For the Tree API, see [tree.cpp](https://github.com/aesophor/Wmderland/blob/master/src/tree.cpp) and [tree_node.cpp](https://github.com/aesophor/Wmderland/blob/master/src/tree_node.cpp)
* For the Tiling Algorithm, see [workspace.cpp](https://github.com/aesophor/Wmderland/blob/master/src/workspace.cpp)

## Build Requirements
* g++ (requires C++11)
* CMake
* Xlib headers
* [glog](https://github.com/google/glog) (Google's C++ logging library)

## Installation
1. Clone the repo
```
$ git clone https://github.com/aesophor/Wmderland
$ cd Wmderland
```

2. Build from source (requires **GLOG**, see Build Requirements)
```
$ mkdir build
$ cmake .. -DCMAKE_BUILD_TYPE=MINSIZEREL
$ make
$ sudo make install
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
* Web browser windows should go fullscreen as video goes fullscreen
* WINE Steam is extremely buggy
* Two borders

## Other Screenshots
Floating windows
![](https://github.com/aesophor/Wmderland/raw/master/assets/floating.png)

## Special Thanks
* [Markus Elfring](https://github.com/elfring) for giving me lots of helpful advice
* Airblader's [i3wm](https://github.com/i3/i3)
* suckless.org's [dwm](https://dwm.suckless.org/)
* jichu4n's [basic_wm](https://github.com/jichu4n/basic_wm)
* baskerville's [bspwm](https://github.com/baskerville/bspwm)
* mil's [simple_wm](https://github.com/mil/simple-wm)
* JLErvin's [berry](https://github.com/JLErvin/berry)
* Mr messyhair's [eveningwm](https://gitlab.com/mrmessyhair/eveningwm/blob/master/eveningwm.c)

## License
Available under the [MIT License](https://github.com/aesophor/Wmderland/blob/master/LICENSE)
