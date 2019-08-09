// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef WMDERLAND_WORKSPACE_H_
#define WMDERLAND_WORKSPACE_H_

extern "C" {
#include <X11/Xlib.h>
}
#include <memory>
#include <vector>

#include "config.h"
#include "client.h"
#include "tree.h"
#include "util.h"

namespace wmderland {

class Client;

class Workspace {
 public:
  Workspace(Display* dpy, Window root_window_, Config* config, int id);
  virtual ~Workspace() = default;

  bool Has(Window window) const;
  void Add(Window window, bool floating=false) const;
  void Remove(Window window) const;
  void Move(Window window, Workspace* new_workspace) const;
  void Arrange(const Area& tiling_area) const;
  void SetTilingDirection(tiling::Direction tiling_direction) const;

  void MapAllClients() const;
  void UnmapAllClients() const;
  void RaiseAllFloatingClients() const;
  void SetFocusedClient(Window window) const;
  void UnsetFocusedClient() const;

  void Focus(Action::Type focus_action_type) const;
  Client* GetFocusedClient() const;
  Client* GetClient(Window window) const;
  std::vector<Client*> GetClients() const;
  std::vector<Client*> GetFloatingClients() const;
  std::vector<Client*> GetTilingClients() const;

  Config* config() const;
  int id() const;
  const char* name() const;
  bool is_fullscreen() const;
  void set_fullscreen(bool fullscreen);

 private:
  void Tile(Tree::Node* node, int x, int y, int width, int height,
            int border_width, int gap_width) const;
  void FocusLeft() const;
  void FocusRight() const;
  void FocusUp() const;
  void FocusDown() const;

  Display* dpy_;
  Window root_window_;
  Config* config_;
  std::unique_ptr<Tree> client_tree_;

  int id_;
  std::string name_;
  bool is_fullscreen_;
};

} // namespace wmderland

#endif // WMDERLAND_WORKSPACE_H_
