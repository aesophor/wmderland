#ifndef WORKSPACE_HPP_
#define WORKSPACE_HPP_

#include <X11/Xlib.h>
#include <algorithm>
#include <vector>
 
// A Workspace contains its id, the active window in this workspace
// and a list of windows.
class Workspace {
public:
    Workspace(Display* dpy, short id);

    void MapAllWindows();
    void UnmapAllWindows();
    
    void Add(const Window w);
    void Remove(const Window w);
    bool Has(const Window w);

    short id();
    Window active_window();

private:
    Display* dpy_;
    short id_;
    Window active_window_;
    std::vector<Window> windows_;
};

#endif
