# Hacking Guide
This README will guide you to hack wmderland. I'll make everything short and simple.

<br>

## Table of Contents
* [Core Concepts](https://github.com/aesophor/wmderland/blob/master/HACKING.md#core-concepts)
* [Internal Data Structures](https://github.com/aesophor/wmderland/blob/master/HACKING.md#internal-data-structures)
* [Windows? Clients?](https://github.com/aesophor/wmderland/blob/master/HACKING.md#windows-clients)
* [Memory Management](https://github.com/aesophor/wmderland/blob/master/HACKING.md#memory-management)
* [Questions?](https://github.com/aesophor/wmderland/blob/master/HACKING.md#questions)

<br>

## Core Concepts
How window managers work:
* Each window shown on your screen has a unique ID (long int)
* We can call XMoveResizeWindow() to manage the positions and sizes of windows

Now a simple floating window manager is done. To add workspace features:
* Maintain a list of Workspace objects
* Since each workspace may contains zero, one or multiple windows,
we'll need some data structures to keep track of the windows living in each workspace.
* The simplest method is to use a 2D list, but this is not flexible.
* wmderland use trees (The most basic ones). See the next section for details.

<br>

## Internal Data Structures
Windows are represented as the leaves of a tree, while the internal nodes tell
the spliting direction of its children. The root node always exists with the client tree
(it WON'T and MUSTN'T be deleted). The windows are XMoveResizeWindow()ed recursively.

The following illustration shows how wmderland stores the window layouts using trees,
where R is the root node, V means its children will be tiled vertically, and H means
its children will be tiled horizontally.

```
             R                         R                          R
                                      /                          / \
                        --->         1             --->         1   2
                                     ^                              ^

+-----------------------+  +-----------------------+  +-----------------------+
|                       |  |                       |  |           |           |
|                       |  |                       |  |           |           |
|                       |  |                       |  |           |           |
|           R           |  |           1           |  |     1     |     2     |
|                       |  |           ^           |  |           |     ^     |
|                       |  |                       |  |           |           |
|                       |  |                       |  |           |           |
+-----------------------+  +-----------------------+  +-----------------------+

          Empty                      Spawn 1                   Spawn 2 

```

```
            R                          R                           R
           /|\                        /|\                         /|\
          1 2 3         --->         1 2 V         --->          1 2 V
              ^                         / \                         / \
                                       3   4                       3   H
                                           ^                          / \
                                                                     4   5
                                                                         ^

+-----------------------+  +-----------------------+  +-----------------------+
|       |       |       |  |       |       |       |  |       |       |       |
|       |       |       |  |       |       |   3   |  |       |       |   3   |
|       |       |       |  |       |       |       |  |       |       |       |
|   1   |   2   |   3   |  |   1   |   2   |-------|  |   1   |   2   |-------|
|       |       |   ^   |  |       |       |       |  |       |       |   |   |
|       |       |       |  |       |       |   4   |  |       |       | 4 | 5 |
|       |       |       |  |       |       |   ^   |  |       |       |   | ^ |
+-----------------------+  +-----------------------+  +-----------------------+

         Spawn 3               $Mod+V and Spawn 4        $Mod+G and Spawn 5
         
```

* For Tree API, see [tree.cc](https://github.com/aesophor/wmderland/blob/master/src/tree.cc)
* For Window management algorithms, see [workspace.cc](https://github.com/aesophor/wmderland/blob/master/src/workspace.cc)

<br>

## Windows? Clients?
TL;DR: `Client` is a wrapper class for `Window`.

Xlib is written in C and its syntax is not object-oriented enough. For example
(you don't have to care about the function arguments for now, just focus on the syntax)
```
XMapWindow(dpy, window_id);
XMoveResizeWindow(dpy, window_id, x, y, width, height);
```

The `Client` class makes the above syntax clearer:
```
Client c(dpy, window_id, residing_workspace);

c.Map();
c.MoveResize(x, y, width, height);
```

Each Xlib function which operates on `Window`s should have a corresponding method in `Client` class.
Prefer these C++ method over the ones provided by Xlib. If you can't find a method in `Client`, you may add it yourself.

<br>

## Memory Management
We mostly use `unique_ptr` to manage the lifetime of objects (e.g., Tree::Node, Client, Workspace, Config, etc)
as there is no need for `shared_ptr` currently.

<br>

## Questions?
If you want to start hacking but don't know where to start, please open an issue. I'm always open to discussions and changes!
