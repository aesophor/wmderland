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

## Wmderland (Pronounced "Wonderland")
A minimal i3-gaps, written in C++ and Xlib.

<a href="https://raw.githubusercontent.com/aesophor/wmderland/master/.meta/floating.png"><img src="https://raw.githubusercontent.com/aesophor/wmderland/master/.meta/floating.png" width="43%" align="right"></a>

* Configurable workflow (keyboard, mouse, or both)
* i3-like [config](https://github.com/aesophor/wmderland/blob/master/example/config); easy to use!
* 9 Workspaces (static)
* Fullscreen toggle
* Floating toggle
* Gaps & borders
* Window focus follows mouse
* Smart floating
* Supports [a subset of EWMH](https://github.com/aesophor/wmderland/blob/master/src/properties.cc)
* [Tiny IPC Client](https://github.com/aesophor/wmderland/tree/master/ipc-client)

## Advantages over i3wm and i3-gaps
wmderland, while derived from i3wm, is built with a different philosophy in mind:

* No bloated (unused) features!
* Easy-to-use config
* Only 88KB after compiled and stripped
* It's designed for a minimalist like you

## Installation
| Distro | Source | Command |
| --- | --- | --- |
| Arch | [AUR](https://aur.archlinux.org/packages/wmderland-git/) | `yay -S wmderland-git` |
| Gentoo | [aesophor-overlay](https://github.com/aesophor/aesophor-overlay) | `emerge -av x11-wm/wmderland` |
| Others | build manually | See [BUILD.md](https://github.com/aesophor/wmderland/blob/master/BUILD.md) |

## Post-Installation
After installation, remember to place a config file ([example](https://github.com/aesophor/wmderland/blob/master/example/config)) at **~/.config/wmderland/config**.

Before starting wmderland, [EDIT your config first!](https://github.com/aesophor/wmderland/blob/master/BUILD.md#configure-and-run) Or you'll get a blackscreen.

## License
Available under the [MIT License](https://github.com/aesophor/wmderland/blob/master/LICENSE)
