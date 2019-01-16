// A Workspace contains its id, the active window in this workspace
// and a list of windows.

#ifndef WORKSPACE_HPP_
#define WORKSPACE_HPP_

extern "C" {
#include <X11/Xlib.h>
}
#include <vector>
#include "client.hpp"
#include "tiling.hpp"
#include "tree.hpp"
#include "util.hpp"

class Client;

class Workspace {
public:
    Workspace(Display* dpy, Window root_window_, int id);
    virtual ~Workspace();

    bool Has(Window w);
    void Add(Window w, bool is_floating);
    void Remove(Window w);
    void Move(Window w, Workspace* new_workspace);
    void Arrange(int bar_height);
    void SetTilingDirection(tiling::Direction tiling_direction);

    void MapAllClients();
    void UnmapAllClients();
    void RaiseAllFloatingClients();
    void SetFocusedClient(Window w);
    void UnsetFocusedClient();

    Client* GetFocusedClient() const;
    Client* GetClient(Window w) const;
    std::vector<Client*> GetFloatingClients() const;
    std::vector<Client*> GetTilingClients() const;

    void FocusLeft();
    void FocusRight();
    void FocusUp();
    void FocusDown();
    
    int id();
    bool is_fullscreen();
    void set_fullscreen(bool is_fullscreen);

private:
    void Tile(TreeNode* node, int x, int y, int width, int height);

    Display* dpy_;
    Window root_window_;
    Tree* client_tree_;
    
    int id_;
    bool is_fullscreen_;
};

#endif
