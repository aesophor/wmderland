// A Workspace contains its id, the active window in this workspace
// and a list of windows.

#ifndef WMDERLAND_WORKSPACE_HPP_
#define WMDERLAND_WORKSPACE_HPP_

#include "client.hpp"
#include "tree.hpp"
#include "util.hpp"
extern "C" {
#include <X11/Xlib.h>
}
#include <memory>
#include <vector>

class Client;

class Workspace {
public:
    Workspace(Display* dpy, Window root_window_, int id);
    virtual ~Workspace();

    bool Has(Window w) const;
    void Add(Window w, bool is_floating) const;
    void Remove(Window w) const;
    void Move(Window w, Workspace* new_workspace) const;
    void Arrange(const Area& tiling_area) const;
    void SetTilingDirection(tiling::Direction tiling_direction) const;

    void MapAllClients() const;
    void UnmapAllClients() const;
    void RaiseAllFloatingClients() const;
    void SetFocusedClient(Window w) const;
    void UnsetFocusedClient() const;

    Client* GetFocusedClient() const;
    Client* GetClient(Window w) const;
    std::vector<Client*> GetFloatingClients() const;
    std::vector<Client*> GetTilingClients() const;

    void FocusLeft() const;
    void FocusRight() const;
    void FocusUp() const;
    void FocusDown() const;
    
    int id() const;
    const char* name() const;
    bool is_fullscreen() const;
    void set_fullscreen(bool is_fullscreen);

private:
    void Tile(TreeNode* node, int x, int y, int width, int height, int border_width, int gap_width) const;

    Display* dpy_;
    Window root_window_;
    std::unique_ptr<Tree> client_tree_;
    
    int id_;
    std::string name_;
    bool is_fullscreen_;
};

#endif
