// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "client.h"

#include "config.h"
#include "workspace.h"
#include "util.h"

using std::string;
using std::unordered_map;

namespace wmderland {

unordered_map<Window, Client*> Client::mapper_;

Client::Client(Display* dpy, Window window, Workspace* workspace)
    : dpy_(dpy),
      window_(window),
      workspace_(workspace),
      size_hints_(wm_utils::GetWmNormalHints(window)),
      attr_cache_(),
      is_floating_(),
      is_fullscreen_(),
      has_unmap_req_from_user_() {
  Client::mapper_[window] = this;
  SetBorderWidth(workspace->config()->border_width());
  SetBorderColor(workspace->config()->unfocused_color());
}

Client::~Client() {
  Client::mapper_.erase(window_);
}


XWindowAttributes Client::GetXWindowAttributes() const {
  return wm_utils::GetXWindowAttributes(window_);
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

const XWindowAttributes& Client::attr_cache() const {
  return attr_cache_;
}


bool Client::is_floating() const {
  return is_floating_;
}

bool Client::is_fullscreen() const {
  return is_fullscreen_;
}

bool Client::has_unmap_req_from_user() const {
  return has_unmap_req_from_user_;
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

void Client::set_has_unmap_req_from_user(bool has_unmap_req_from_user) {
  has_unmap_req_from_user_ = has_unmap_req_from_user;
}

void Client::set_attr_cache(const XWindowAttributes& attr) {
  attr_cache_ = attr;
}


Client::Area::Area() : x(), y(), w(), h() {}

Client::Area::Area(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

bool Client::Area::operator== (const Client::Area& other) {
  return (x == other.x) && (y == other.y) && (w== other.w) && (h== other.h);
}

bool Client::Area::operator!= (const Client::Area& other) {
  return (x != other.x) || (y != other.y) || (w!= other.w) || (h!= other.h);
}

} // namespace wmderland
