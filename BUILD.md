## Build Requirements / Installation
#### Build Requirements
* g++ (requires C++14)
* CMake
* Xlib headers
* **Optional** - [glog](https://github.com/google/glog) (Google's C++ logging library)

<br>

#### Installation
1. Build and install project (executables will be installed under /usr/local/bin/)
```
$ git clone https://github.com/aesophor/wmderland.git
$ cd wmderland
$ ./build.sh -i
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
