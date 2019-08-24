Wmderland Client
---

A tiny program to interact with Wmderland

Build
---
You can run `build.sh` from the top-level directory to build this project, or

```
$ gcc -o Wmderlandc wmderlandc.c -lX11
```

Usage
---
```
$ Wmderlandc navigate_left
$ Wmderlandc navigate_right
$ Wmderlandc navigate_down
$ Wmderlandc navigate_up
$ Wmderlandc tile_h
$ Wmderlandc tile_v
$ Wmderlandc toggle_floating
$ Wmderlandc toggle_fullscreen
$ Wmderlandc goto_workspace <number>
$ Wmderlandc move_window_to_workspace <number>
$ Wmderlandc kill # kill current window
$ Wmderlandc exit # exit WM
$ Wmderlandc reload # reload config
$ Wmderlandc debug_crash # don't use this
```
