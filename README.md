<div align="center">

<h3>WMDERLAND</h3>
<p>Modern and Minimal X11 Tiling Window Manager</p>

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
wmderland is a modern and minimal Tiling Window Manager developed in C/C++ with [Xlib](https://en.wikipedia.org/wiki/Xlib)

* A lightweight i3-gaps, written from scratch
* An easy-to-use config system ([example](https://github.com/aesophor/wmderland/blob/master/example/config))
* Stable, simple, and maintainable

**Documentation available at its [wiki page](https://github.com/aesophor/wmderland/wiki).**

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
* tiling behaviors are similar to that of i3wm
* written in C++ OOP and Xlib. Only ~3k lines of C++14 code, hopefully it will be a lot easier to maintain.

<br>

## What's Included?
* Tree-based horizontal and vertical tiling (like i3wm)
* Gaps and borders (like i3-gaps)
* Statically allocated workspaces (i3wm has dynamically allocated workspaces)
* Smart floating (dialog windows will be floating by default, their pos/size will be remembered)
* Easy-to-use [config](https://github.com/aesophor/wmderland/blob/master/example/config) with runtime reload support
* Supports a subset of EWMH, see `src/properties.cc`
* Shipped with a [tiny client](https://github.com/aesophor/wmderland/tree/master/ipc-client) which can interact/control wmderland

<br>

## Installation
Remember to place a config file ([example](https://github.com/aesophor/wmderland/blob/master/example/config)) at **~/.config/wmderland/config** after installation.

* Arch Linux (AUR): `yay -S wmderland-git`

* Gentoo Linux (aesophor-overlay): `emerge -av x11-wm/wmderland` ([Add my overlay first](https://github.com/aesophor/aesophor-overlay))

<br>

## Build Manually
See [BUILD.md](https://github.com/aesophor/wmderland/blob/master/BUILD.md).

<br>

## Upgrading from 1.0.2 or earlier
Since 1.0.3, Wmderland has been renamed to `w`mderland. The following files have been renamed:
```
* ~/.config/Wmderland/config
* ~/.cache/Wmderland/cookie
* /usr/local/bin/Wmderland
* /usr/local/bin/Wmderlandc
* /usr/share/xsessions/Wmderland.desktop
```

<br>

## License
Available under the [MIT License](https://github.com/aesophor/wmderland/blob/master/LICENSE)
