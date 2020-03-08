## Build Requirements
* g++ >= 6.0
* CMake >= 3.9
* libx11 (X11 headers)

<br>

## Build and Install
1. Build and install project (installed to /usr/local/bin/)
```
$ git clone https://github.com/aesophor/wmderland.git
$ cd wmderland && ./build.sh --install
```

2. Copy the config file (**IMPORTANT**)
```
$ mkdir -p ~/.config/wmderland
$ cp example/config ~/.config/wmderland/.
```

3. If using a Display Manager, copy the .desktop file
```
$ sudo cp example/wmderland.desktop /usr/share/xsessions/.
```

4. If not using a Display Manager, add this to your ~/.xinitrc and execute `startx`
```
exec wmderland
```

<br>

## Configure and Run
Before you start wmderland, you'll need to modify the config file (~/.config/wmderland/config) first, otherwise you'll get a black screen. See the next section on how to escape from the black screen.

The syntax are very similar to that of i3wm:
* [keybindings](https://github.com/aesophor/wmderland/blob/master/example/config#L166)
* [autostart](https://github.com/aesophor/wmderland/blob/master/example/config#L177)

For example, to set a wallpaper using `feh`:
```
exec feh --bg-fill ~/Pictures/Wallpaper.jpg
```

<br>

## Escape From the Black Screen
In case you are stucked in a black screen, you can try **ONE** of the following:
* **Mod (Windows/Command key) + Shift + Esc**: the keybinding to exit the window manager

* **Ctrl + Alt + F2 and run `pkill X`, and Ctrl + Alt + F1**
