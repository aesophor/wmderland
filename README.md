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
1. From source
```
$ git clone https://github.com/aesophor/Wmderland
$ cd Wmderland && make && sudo make install
```

2. Put the following lines at the END of your ~/.xinitrc, make sure you have only one exec statement (don't invoke other wm!)
```
# Fix Non-reparenting window managers / Grey window /
# Programs not drawing properly
export _JAVA_AWT_WM_NONREPARENTING=1

# Startup Wmderland
exec Wmderland
```

3. Start xorg
```
$ startx
```

## Features
* Horizontal and vertical tiling (with gaps!)
* Toggle windows between tiled / floating.
* Smart floating (dialog windows will be floating by default).
* Workspaces `Super + {1~9}`: switch; `Alt + {1~9}`: move application to...
* Window border.
* A very basic configuration system. ([example config](https://github.com/aesophor/Wmderland/blob/master/example/config))
* Compatible with Polybar's `xwindow` module.

## Todo List
* Terminate client peacefully via ICCCM instead of XKillClient().
* A better configuration system that supports
  * keybinds
  * floating rules with wm_name
  * application spawning rules with wm_name

## Windows Version
Just fook off

## Special Thanks
* suckless.org's dwm
* jichu4n's [basic_wm](https://github.com/jichu4n/basic_wm)
* mil's [simple_wm](https://github.com/mil/simple-wm)
* JLErvin's [berry](https://github.com/JLErvin/berry)

## License
Available under the [MIT License](https://github.com/aesophor/Wmderland/blob/master/LICENSE)

