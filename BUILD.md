## Table of Contents
* Build, Install and Running
  * [build requirements](https://github.com/aesophor/wmderland/blob/master/BUILD.md#build-requirements)
  * [build and install](https://github.com/aesophor/wmderland/blob/master/BUILD.md#build-and-install)
  * [black screen?](https://github.com/aesophor/wmderland/blob/master/BUILD.md#black-screen)
  * [configure and run](https://github.com/aesophor/wmderland/blob/master/BUILD.md#configure-and-run)
  
* Workarounds
  * [escape from the black screen](https://github.com/aesophor/wmderland/blob/master/BUILD.md#escape-from-the-black-screen)
  * [GDM entry not present (Fedora Workstation 31)](https://github.com/aesophor/wmderland/blob/master/BUILD.md#gdm-entry-not-present-fedora-workstation-31)

<br>

## Build Requirements
* g++ >= 6.0
* CMake >= 3.9
* X11 headers

| Distro | X11 headers package name |
| --- | --- |
| Ubuntu | libx11-dev |
| Fedora | libX11-devel |
| Arch | libx11 |
| Gentoo | libX11 |

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
$ cp /etc/xdg/wmderland/config ~/.config/wmderland/.
```

3. If not using a Display Manager, add this to your ~/.xinitrc and execute `startx`
```
exec wmderland
```

<br>

## Black Screen?
Before you start wmderland, you'll need to modify the config file (~/.config/wmderland/config) first, otherwise you'll get a black screen. [How do I escape from the black screen!?](https://github.com/aesophor/wmderland/blob/master/BUILD.md#escape-from-the-black-screen)

<br>

## Configure and Run
Edit `~/.config/wmderland/config`. The syntax is very similar to that of i3wm:
* [autostart](https://github.com/aesophor/wmderland/blob/master/example/config#L177)
* [keybindings](https://github.com/aesophor/wmderland/blob/master/example/config#L166)

For example, to set a wallpaper using `feh`:
```
exec feh --bg-fill ~/Pictures/Wallpaper.jpg
```

To bind Mod+d to `rofi -show drun`:
```
bindsym $Mod+d exec rofi -show drun
```

By default, the Mod key is the Windows/Command key, but this option [can be configured here](https://github.com/aesophor/wmderland/blob/master/example/config#L31):
```
set $Mod = $Cmd
```

<br>

## Escape From the Black Screen
In case you are stucked in a black screen, you can try **ONE** of the following:
* **Mod (Windows/Command key) + Shift + Esc**: the keybinding to exit the window manager

* **Ctrl + Alt + F2 and run `pkill X`, and Ctrl + Alt + F1**

<br>

## GDM Entry Not Present (Fedora Workstation 31)

If wmderland's entry just won't show up in GDM even after reboot, you can try to:
1. autoremove GDM (this will break GNOME a bit, but it can be restored, see step 3)
2. reinstall gdm
3. reinstall gnome-desktop
4. remove wmderland (./build.sh --uninstall)
4. reboot
5. recompile and reinstall wmderland (./build.sh --install)
```
$ sudo systemctl disable gdm.service
$ sudo dnf autoremove -y gdm
$ sudo dnf upgrade -y
$ sudo dnf install -y gdm
$ sudo dnf group install -y gnome-desktop
$ sudo systemctl enable gdm.service

# Now cd to wmderland's project directory
$ ./build.sh --uninstall
$ sudo reboot

# After rebooting, cd to wmderland's project directory
$ rm -rf build
$ ./build.sh --install
```

For further details, please see [issue #39](https://github.com/aesophor/wmderland/issues/39).
