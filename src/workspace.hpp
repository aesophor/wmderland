#ifndef WORKSPACE_HPP_
#define WORKSPACE_HPP_

#include "client.hpp"
#include <X11/Xlib.h>
#include <vector>
 
// A Workspace contains its id, the active window in this workspace
// and a list of windows.
class Workspace {
public:
    Workspace(Display* dpy, short id);
    ~Workspace();

    /* clients_ vector manipulation */
    void Add(Window w);
    void Remove(Window w);
    bool Has(Window w);
    Client* Get(Window w);
    std::string ToString();

    /* client window manipulation */
    void MapAllClients();
    void UnmapAllClients();
    void SetFocusClient(Window focused_window);
    
    short id();
    short active_client();

private:
    Display* dpy_;
    short id_;
    
    short active_client_;
    std::vector<Client*> clients_;
};

#endif
