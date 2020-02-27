// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_WORKSPACE_H_
#define WMDERLAND_WORKSPACE_H_

extern "C" {
#include <X11/Xlib.h>
}
#include <string>
#include <vector>

#include "client.h"
#include "config.h"
#include "tree.h"

namespace wmderland {

class Workspace {
 public:
  Workspace(Display* dpy, Window root_window_, Config* config, int id);
  virtual ~Workspace() = default;

  bool Has(Window window) const;
  void Add(Window window);
  void Remove(Window window);
  void Move(Window window, Workspace* new_workspace);
  void Tile(const Client::Area& tiling_area) const;
  void SetTilingDirection(TilingDirection tiling_direction);

  void MapAllClients() const;
  void UnmapAllClients(Window except_window = None) const;
  void RaiseAllFloatingClients() const;
  void SetFocusedClient(Window window);
  void UnsetFocusedClient() const;

  void DisableFocusFollowsMouse() const;
  void EnableFocusFollowsMouse() const;

  void Navigate(Action::Type navigate_action_type);
  Client* GetFocusedClient() const;
  Client* GetClient(Window window) const;
  std::vector<Client*> GetClients() const;
  std::vector<Client*> GetFloatingClients() const;
  std::vector<Client*> GetTilingClients() const;

  Config* config() const;
  int id() const;
  const char* name() const;
  bool is_fullscreen() const;

  void set_name(const std::string& name);
  void set_fullscreen(bool fullscreen);

  std::string Serialize() const;
  void Deserialize(std::string data);

 private:
  void DfsTileHelper(Tree::Node* node, int x, int y, int w, int h, int border_width,
                     int gap_width) const;

  Display* dpy_;
  Window root_window_;
  Config* config_;
  Tree client_tree_;

  int id_;
  std::string name_;
  bool is_fullscreen_;
};

}  // namespace wmderland

#endif  // WMDERLAND_WORKSPACE_H_
