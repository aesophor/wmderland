<div align="center">
<h3>Wmderland</h3>
<img src="https://github.com/aesophor/Wmderland/raw/master/assets/scrot.jpg">
Made during <a href="https://www.facebook.com/events/256671588330840/">NTUOSC Hackathon 2018</a>
</div>

## Overview
Heavily Inspired by [i3wm](https://github.com/i3/i3). Wmderland aims to simplify the core functionalities of i3wm, and bundle in the essential features required for a modern but minimal Tiling Window Manager.

Written in C++ using [Xlib](https://en.wikipedia.org/wiki/Xlib) with :heart:

## Why Another WM
I started this project because I want experience with OOP in C++ and Xlib. I know there's already [lots of WM out there](https://wiki.archlinux.org/index.php/Window_manager), but I'm writing this for my personal use and study.

## Build Requirements
* g++
* make
* Xlib headers
* [glog](https://github.com/google/glog) (Google's c++ logging library)

## Installation
1. Build from source
```
$ git clone https://github.com/aesophor/Wmderland
$ cd Wmderland && make && sudo make install
```

2. Put the following lines at the END of your ~/.xinitrc, and make sure there's only one exec statement (don't invoke other wm!)
```
# Fix Non-reparenting window managers / Grey window /
# Programs not drawing properly
export _JAVA_AWT_WM_NONREPARENTING=1

# Startup Wmderland
exec Wmderland
```

3. Initialize an X session.
```
$ startx
```

## Features
* Horizontal and vertical tiling (with gaps!)
* Toggle windows between tiled / floating.
* Smart floating (dialog windows will be floating by default).
* Workspaces. ( `Mod4 + {1~9}`: switch; `Mod4 + Shift + {1~9}`: move application to... )
* Window border.
* Configuration supported. ([example config](https://github.com/aesophor/Wmderland/blob/master/example/config))
* Compatible with Polybar's `xwindow` module.

## Todo List
* IPC mechanisms (to communicate with Polybar and other process)
* Improve performance of programs running under Wine (it's kinda laggy for now, idk why)
* Add the ability to define `Super` key in config instead of hardcoding it to `Mod4`
* Remember the position and size of floating windows
* Two borders

## Windows Version
Just fook off

## Special Thanks
* suckless.org's dwm
* jichu4n's [basic_wm](https://github.com/jichu4n/basic_wm)
* mil's [simple_wm](https://github.com/mil/simple-wm)
* JLErvin's [berry](https://github.com/JLErvin/berry)
* Mr messyhair's [eveningwm](https://gitlab.com/mrmessyhair/eveningwm/blob/master/eveningwm.c)

## License
Available under the [MIT License](https://github.com/aesophor/Wmderland/blob/master/LICENSE)
