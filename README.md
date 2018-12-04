<div align="center">
<h3>Wmderland</h3>
<img src="https://github.com/aesophor/Wmderland/raw/master/assets/scrot.jpg">
Made during <a href="https://www.facebook.com/events/256671588330840/">NTUOSC Hackathon 2018</a>
</div>

## Overview
Heavily Inspired by [i3wm](https://github.com/i3/i3). ~~Wmderland aims to simplify the core functionalities of i3wm, and bundle in the essential features required for a modern but minimal Tiling Window Manager.~~

Okay I failed. It turns out Wmderland's binary is fat (~1.1 MB). But there's no noticeable performance penalties, so as long as you don't mind its size, I guess it's fine.

Written in C++ using [Xlib](https://en.wikipedia.org/wiki/Xlib) with :heart:

## Build Requirements
* g++
* make
* Xlib headers
* [glog](https://github.com/google/glog) (Google's c++ logging library)

## Windows Version
Just fook off

## Features
* Horizontal and vertical tiling (with gaps!)
* Toggle windows between tiled / floating.
* Smart floating (dialog windows will be floating by default).
* Workspaces `Super + {1~9}`: switch; `Alt + {1~9}`: move application to...
* Window border.
* A very basic configuration system. ([example config](https://github.com/aesophor/Wmderland/blob/master/example/config))
* Compatible with Polybar's `xwindow` module.

## What's not working yet
* A better configuration system that supports `keybinds`, `floating rules`, `application spawning rules with wm_name`... etc.
* Terminate client peacefully via ICCCM instead of XKillClient().

## License
Available under the [MIT License](https://github.com/aesophor/Wmderland/blob/master/LICENSE)
