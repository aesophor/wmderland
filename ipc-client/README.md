wmderland Client
---

A tiny program to interact with wmderland

Build
---
You can run `build.sh` from the top-level directory to build this project, or

```
$ gcc -o wmderlandc wmderlandc.c -lX11
```

Usage
---
```
$ wmderlandc navigate_left
$ wmderlandc navigate_right
$ wmderlandc navigate_down
$ wmderlandc navigate_up
$ wmderlandc tile_h
$ wmderlandc tile_v
$ wmderlandc toggle_floating
$ wmderlandc toggle_fullscreen
$ wmderlandc goto_workspace <number>
$ wmderlandc workspace <number>
$ wmderlandc move_window_to_workspace <number>
$ wmderlandc kill # kill current window
$ wmderlandc exit # exit WM
$ wmderlandc reload # reload config
$ wmderlandc debug_crash # don't use this
```
