// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "client.h"

#include "config.h"

using std::string;
using std::unordered_map;

namespace wmderland {

unordered_map<Window, Client*> Client::mapper_;

Client::Client(Display* dpy, Window window, Workspace* workspace)
    : dpy_(dpy),
      window_(window),
      workspace_(workspace),
      size_hints_(wm_utils::GetWmNormalHints(window)),
      previous_attr_(), // this will be set when Client::SaveXWindowAttributes() is called
      is_floating_(false),
      is_fullscreen_(false) {
  Client::mapper_[window] = this;
  SetBorderWidth(workspace->config()->border_width());
  SetBorderColor(workspace->config()->unfocused_color());
}

Client::~Client() {
  Client::mapper_.erase(window_);
}


void Client::SaveXWindowAttributes() {
  previous_attr_ = wm_utils::GetXWindowAttributes(window_);
}


Window Client::window() const {
  return window_;
}

Workspace* Client::workspace() const {
  return workspace_;
}

const XSizeHints& Client::size_hints() const {
  return size_hints_;
}

const XWindowAttributes& Client::previous_attr() const {
  return previous_attr_;
}


bool Client::is_floating() const {
  return is_floating_;
}

bool Client::is_fullscreen() const {
  return is_fullscreen_;
}


void Client::set_workspace(Workspace* workspace) {
  workspace_ = workspace;
}

void Client::set_floating(bool floating) {
  is_floating_ = floating;
}

void Client::set_fullscreen(bool fullscreen) {
  is_fullscreen_ = fullscreen;
}

} // namespace wmderland
