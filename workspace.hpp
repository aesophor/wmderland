#ifndef WORKSPACE_HPP_
#define WORKSPACE_HPP_

#include <X11/Xlib.h>
#include <vector>

struct Workspace {
    std::vector<Window> windows;
    Window active_window;
};

#endif
