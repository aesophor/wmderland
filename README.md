<div align="center">
<h3>Wmderland</h3>
<img src="https://github.com/aesophor/Wmderland/raw/master/assets/scrot.jpg">
Made during <a href="https://www.facebook.com/events/256671588330840/">NTUOSC Hackathon 2018</a>
</div>

## Overview
Heavily Inspired by [i3wm](https://github.com/i3/i3). Wmderland aims to simplify the core functionalities of i3wm, and bundle in the essential features required for a modern but minimal Tiling Window Manager.

Written in C++ using [Xlib](https://en.wikipedia.org/wiki/Xlib). This project is still in very early stage of development, so there are lots of bugs.

## Build Requirements
* g++
* make
* Xlib headers
* [glog](https://github.com/google/glog) (Google's c++ logging library)

## Windows Version
Just fook off

## What's not working yet
* Smart floating like in i3wm
* Switch between tiling and floating
* User Configuration for defining keybinds, startup applications, colors... etc.
* Terminate client peacefully via ICCCM.

## What's working
Super is hardcoded to `Mod4` (Win/Cmd) for now. This will be improved later.
* Simple window tiling. `Super+v` vertical mode; `Super+g` horizontal mode.
* Switch between workspaces `Super + 1~9`
* Move applications between workspaces `Alt + 1~9`
* Window border (focused color / unfocused color)
* Gaps between tiled windows
* Move windows with `Super + MouseLeftBtn`
* Resize windows with `Super + MouseRightBtn`
* Launch urxvt with `Super + Return`
* Launch rofi with `Super + d`
* Kill applications with `Super + q`
* Fullscreen applications with `Super + f` 
* _NET_WM_NAME set to "Wmderland"
* Compatible with Polybar's `xwindow` module (_NET_ACTIVE_WINDOW)
* Recognizes polybar (because I use polybar lol)

## License
Available under the [MIT License](https://github.com/aesophor/Wmderland/blob/master/LICENSE)
