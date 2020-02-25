<div align="center">

<h3>WMDERLAND</h3>
<p>Modern, Minimal X11 Tiling Window Manager</p>

<a href="http://hits.dwyl.io/aesophor/Wmderland">
  <img src="http://hits.dwyl.io/aesophor/Wmderland.svg">
</a>
<a href="https://github.com/aesophor/wmderland/blob/master/LICENSE">
  <img src="https://img.shields.io/badge/license-MIT-brightgreen.svg">
 </a>
<a href="https://travis-ci.org/aesophor/wmderland">
  <img src="https://travis-ci.org/aesophor/wmderland.svg?branch=master">
</a>

<img src="https://github.com/aesophor/wmderland/raw/master/.meta/tiling.png">
</div>

<br>

## Overview
wmderland is a modern, minimal Tiling Window Manager written in C/C++ with [Xlib](https://en.wikipedia.org/wiki/Xlib)

* A lightweight i3, written from scratch
* An easy-to-use config system ([example](https://github.com/aesophor/wmderland/blob/master/example/config))
* Stable, simple, and maintainable

**Documentation available at its [wiki page](https://github.com/aesophor/wmderland/wiki).**

<br>

## Build Manually
See [BUILD.md](https://github.com/aesophor/wmderland/blob/master/BUILD.md).

<br>

## Installation
* Arch Linux
```
yay -S wmderland-git
```

<br>

## Post-Installation
Remember to place a config file at `~/.config/wmderland/config`

[An example config is available here](https://github.com/aesophor/wmderland/blob/master/example/config).

<br>

## Philosophy / Why Use This?
#### i3wm
A versatile window manager which is shipped with i3bar, has a nice tiling algorithm, several window layout modes (default, stacking, tabbed) and it is very stable. However, I think certain areas can be further improved:
* config system is too complicated
* there are many features that I've rarely used
* ~44k lines of C code, which is a bit hard to trace and maintain

#### wmderland
wmderland, while derived from i3, is built with a different philosophy in mind. The plan for wmderland is to become a modern but minimal tiling window manager which:
* follows Unix philosophy - Do one thing and do it well
* has an easy-to-use config
* tiling behaviors are similar to that of i3
* written in C++ OOP and Xlib. Only ~3k lines of C++14 code, hopefully it will be a lot easier to maintain.

<br>

## What's Included?
* Tree-based horizontal and vertical tiling (like i3)
* Gaps and borders (like i3-gaps)
* Statically allocated workspaces (i3 has dynamically allocated workspaces)
* Smart floating (dialog windows will be floating by default, their pos/size will be cached)
* Easy-to-use [config](https://github.com/aesophor/wmderland/blob/master/example/config) with runtime reload support
* Supports a subset of EWMH, see `src/properties.cc`
* Error recovery mechanism
* Shipped with a [tiny client](https://github.com/aesophor/wmderland/tree/master/ipc-client) which can interact/control wmderland

<br>

## Special Thanks
![](https://github.com/aesophor/wmderland/raw/master/.meta/floating.png)

* [Markus Elfring](https://github.com/elfring) for giving me lots of helpful advice
* Airblader's [i3wm](https://github.com/i3/i3)
* suckless.org's [dwm](https://dwm.suckless.org/)
* jichu4n's [basic_wm](https://github.com/jichu4n/basic_wm)
* baskerville's [bspwm](https://github.com/baskerville/bspwm)
* mil's [simple_wm](https://github.com/mil/simple-wm)
* JLErvin's [berry](https://github.com/JLErvin/berry)
* Mr messyhair's [eveningwm](https://gitlab.com/mrmessyhair/eveningwm/blob/master/eveningwm.c)

<br>

## License
Available under the [MIT License](https://github.com/aesophor/wmderland/blob/master/LICENSE)
