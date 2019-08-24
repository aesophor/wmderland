<div align="center">
<h3>Wmderland</h3>
Project started in <a href="https://www.facebook.com/events/256671588330840/">NTUOSC Hackathon 2018</a>
<img src="https://github.com/aesophor/Wmderland/raw/master/.meta/tiling.png">
</div>

## Overview
Wmderland is a **tiling window manager** written in C/C++ with [Xlib](https://en.wikipedia.org/wiki/Xlib)

* A lightweight version of i3wm, written from scratch
* An easy-to-use configuration system ([see example](https://github.com/aesophor/Wmderland/blob/master/example/config))
* Stable, simple and maintainable

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
|                       |  |                       |  |           |           |
|           R           |  |           1           |  |     1     |     2     |
|                       |  |           ^           |  |           |     ^     |
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
|       |       |       |  |       |       |       |  |       |       |       |
|   1   |   2   |   3   |  |   1   |   2   |-------|  |   1   |   2   |-------|
|       |       |   ^   |  |       |       |       |  |       |       |   |   |
|       |       |       |  |       |       |   4   |  |       |       | 4 | 5 |
|       |       |       |  |       |       |   ^   |  |       |       |   | ^ |
+-----------------------+  +-----------------------+  +-----------------------+

         Spawn 3               $Mod+V and Spawn 4        $Mod+G and Spawn 5
         
```

* For Tree API, see [tree.cc](https://github.com/aesophor/Wmderland/blob/master/src/tree.cc)
* For Window management algorithms, see [workspace.cc](https://github.com/aesophor/Wmderland/blob/master/src/workspace.cc)

## Build Requirements
* g++ (requires C++11)
* CMake
* Xlib headers
* **Optional** - [glog](https://github.com/google/glog) (Google's C++ logging library)

## Installation
1. Run build.sh and install
```
$ git clone https://github.com/aesophor/Wmderland.git
$ cd Wmderland
$ ./build.sh -i
```

2. Copy the config file (**IMPORTANT**)
```
$ mkdir -p ~/.config/Wmderland
$ cp example/config ~/.config/Wmderland/.
```

3. If not using a Display Manager, add this to your ~/.xinitrc and execute `startx`
```
exec Wmderland
```

## Main Features
* Tree-based horizontal and vertical tiling (like in i3wm)
* Gaps and borders
* Static workspaces
* Smart floating (dialog windows will be floating by default)
* Easy-to-use [config](https://github.com/aesophor/Wmderland/blob/master/example/config) with runtime reload support
* Supports a subset of EWMH, see `src/properties.cc`
* Remembers the positions/sizes of floating windows (~/.local/share/Wmderland/cookie)
* Error recovery mechanism

## Error Recovery
If Wmderland crashes due to an exception, it will try to perform error recovery from a **snapshot file**.

When a C++ exception is caugh, **except Snapshot::SnapshotLoadError**, it will:
1. serialize all window trees (or client trees) into snapshot file.
2. write all docks/notifications window id(s) into snapshot file.
3. execl(args[0], args[0])
4. try to deserialize everything from the snapshot.

Also, if it fails to load the snapshot, or the same error occurs consecutively over 3 times,
wmderland will still shutdown. The user will have to manually restart it.

## Compatibility with WINE
Most WINE applications should work fine, except the following bugs:
* ~~WINE applications will hang on close~~ **fixed in commit [`a816f31`](https://github.com/aesophor/Wmderland/commit/a816f312d4f6b06865d36bbb565be95475d71719#comments)**
* WINE Steam floating menu windows are slightly laggy when closed
* WINE Steam startup logo window is not correctly resized

## Todo List
* Add support for [sxhkd](https://github.com/baskerville/sxhkd)
* Web browser windows should go fullscreen as video goes fullscreen
* Rounded corners
* Two borders

## Other Screenshots
Floating windows
![](https://github.com/aesophor/Wmderland/raw/master/.meta/floating.png)

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
