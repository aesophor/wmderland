#ifndef WORKSPACE_HPP_
#define WORKSPACE_HPP_

#include "client.hpp"
#include <X11/Xlib.h>
#include <algorithm>
#include <vector>
 
// A Workspace contains its id, the active window in this workspace
// and a list of windows.
class Workspace {
public:
    Workspace(Display* dpy, short id);

    void Add(Client* c);
    void Remove(Window w);
    bool Has(Window w);
    Client* Get(Window w);
    std::string ToString();

    void MapAllWindows();
    void UnmapAllWindows();
     
    short id();
    Client* active_client();

private:
    Display* dpy_;
    short id_;
    
    Client* active_client_;
    std::vector<Client*> clients_;
};

#endif
