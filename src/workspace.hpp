#ifndef WORKSPACE_HPP_
#define WORKSPACE_HPP_

#include "client.hpp"
#include "util.hpp"
#include <X11/Xlib.h>
#include <vector>
#include <string>

/* A Workspace contains its id, the active window in this workspace
 * and a list of windows.
 */
class Client;

class Workspace {
public:
    Workspace(Display* dpy, short id);
    ~Workspace();

    /* clients_ vector manipulation */    
    void Add(Window w, Direction tiling_direction, bool is_floating);
    void Remove(Window w);
    void Move(Window w, Workspace* workspace);
    bool Has(Window w);
    
    bool IsEmpty();
    short ColSize() const;
    short RowSize(short col_idx) const;
    std::string ToString();
    
    Client* Get(Window w);
    Client* GetByIndex(std::pair<short, short> pos);

    std::vector<Client*> GetFloatingClients();
    std::vector<std::vector<Client*> > GetTilingClients();
    
    /* client window manipulation */
    void MapAllClients();
    void UnmapAllClients();
    void RaiseAllFloatingClients();
    void SetFocusClient(Window focused_window);
    void UnsetFocusClient();
    
    void FocusLeft();
    void FocusRight();
    void FocusUp();
    void FocusDown();
    
    short id();
    Client* active_client();
    std::pair<short, short> active_client_pos();

private:
    void AddHorizontal(Window w, bool is_floating);
    void AddVertical(Window w, bool is_floating);

    Display* dpy_;
    short id_;
    
    std::pair<short, short> active_client_pos_;
    std::vector<std::vector<Client*> > clients_;
};

#endif
