<div align="center">
<h3>Wmderland</h3>
<img src="https://github.com/aesophor/Wmderland/raw/master/assets/scrot.jpg">

</div>

## Overview
A lightweight ~~tiling~~ [window manager](https://en.wikipedia.org/wiki/Window_manager) written in C++ over Xlib. (Currently it's only a floating wm)

## Build Requirements
* g++
* make
* Xlib headers
* [glog](https://github.com/google/glog) (Google's c++ logging library)

## Windows Version
Just fook off

## What's not working yet
* Dynamic Tiling with gaps
* User Configuration for defining keybinds, startup applications, colors... etc.


## What's working
Super is hardcoded to `Mod4` (Win/Cmd) for now. This will be improved later.
* Workspaces `Super + 1~9` 
* Window border (focused color / unfocused color)
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

