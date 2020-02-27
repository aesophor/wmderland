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

3. If not using a Display Manager, add this to your ~/.xinitrc and execute `startx`
```
exec wmderland
```
