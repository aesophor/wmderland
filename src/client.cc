// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#include "client.h"

extern "C" {
#include <X11/extensions/shape.h>
}

#include "config.h"
#include "util.h"
#include "workspace.h"

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
      is_mapped_(),
      is_floating_(),
      is_fullscreen_(),
      has_unmap_req_from_wm_() {
  Client::mapper_[window] = this;
  SetBorderWidth(workspace->config()->border_width());
  SetBorderColor(workspace->config()->unfocused_color());
}

Client::~Client() {
  Client::mapper_.erase(window_);
}

void Client::Map() const {
  XMapWindow(dpy_, window_);
}

void Client::Unmap() {
  // If this client is already unmapped, or the WM has already sent a request
  // to unmap it, then no need to do it again.
  if (!is_mapped_ || has_unmap_req_from_wm_) {
    return;
  }

  has_unmap_req_from_wm_ = true;  // will be set to false in WindowManager::OnUnmapNotify
  XUnmapWindow(dpy_, window_);
}

void Client::Raise() const {
  XRaiseWindow(dpy_, window_);
}

void Client::Move(int x, int y, bool absolute) const {
  if (absolute) {
    XMoveWindow(dpy_, window_, x, y);
    return;
  }

  XWindowAttributes attr = GetXWindowAttributes();
  XMoveWindow(dpy_, window_, attr.x + x, attr.y + y);
}

void Client::Resize(int w, int h, bool absolute) const {
  if (absolute) {
    ConstrainSize(w, h);
    XResizeWindow(dpy_, window_, w, h);
    return;
  }

  // Resize window by relative width and height.
  XWindowAttributes attr = GetXWindowAttributes();
  w = attr.width + w;
  h = attr.height + h;
  ConstrainSize(w, h);
  XResizeWindow(dpy_, window_, w, h);
}

void Client::MoveResize(int x, int y, int w, int h, bool absolute) const {
  if (absolute) {
    ConstrainSize(w, h);
    XMoveResizeWindow(dpy_, window_, x, y, w, h);
    return;
  }

  // Resize window by relative width and height.
  XWindowAttributes attr = GetXWindowAttributes();
  w = attr.width + w;
  h = attr.height + h;
  ConstrainSize(w, h);
  XMoveResizeWindow(dpy_, window_, attr.x + x, attr.y + y, w, h);
}

void Client::MoveResize(int x, int y, const std::pair<int, int>& size) const {
  XMoveResizeWindow(dpy_, window_, x, y, size.first, size.second);
}

void Client::SetInputFocus() const {
  XSetInputFocus(dpy_, window_, RevertToParent, CurrentTime);
}

void Client::SetBorderWidth(unsigned int width) const {
  XSetWindowBorderWidth(dpy_, window_, width);
}

void Client::SetBorderColor(unsigned long color) const {
  XSetWindowBorder(dpy_, window_, color);
}

void Client::SelectInput(long input_mask) const {
  XSelectInput(dpy_, window_, input_mask);
}

XWindowAttributes Client::GetXWindowAttributes() const {
  return wm_utils::GetXWindowAttributes(window_);
}

void Client::Move(const Action& action) const {
  const int& move_step = workspace_->config()->float_move_step();
  int x_offset = 0;
  int y_offset = 0;

  switch (action.type()) {
    case Action::Type::FLOAT_MOVE_LEFT:
      x_offset = -move_step;
      break;
    case Action::Type::FLOAT_MOVE_RIGHT:
      x_offset = move_step;
      break;
    case Action::Type::FLOAT_MOVE_UP:
      y_offset = -move_step;
      break;
    case Action::Type::FLOAT_MOVE_DOWN:
      y_offset = move_step;
      break;
    default:
      break;
  }

  Move(x_offset, y_offset, /*absolute=*/false);
}

void Client::Resize(const Action& action) const {
  const int& resize_step = workspace_->config()->float_resize_step();
  int width_offset = 0;
  int height_offset = 0;

  switch (action.type()) {
    case Action::Type::FLOAT_RESIZE_LEFT:
      width_offset = -resize_step;
      break;
    case Action::Type::FLOAT_RESIZE_RIGHT:
      width_offset = resize_step;
      break;
    case Action::Type::FLOAT_RESIZE_UP:
      height_offset = -resize_step;
      break;
    case Action::Type::FLOAT_RESIZE_DOWN:
      height_offset = resize_step;
      break;
    default:
      break;
  }

  Resize(width_offset, height_offset, /*absolute=*/false);
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

bool Client::is_mapped() const {
  return is_mapped_;
}

bool Client::is_floating() const {
  return is_floating_;
}

bool Client::is_fullscreen() const {
  return is_fullscreen_;
}

bool Client::has_unmap_req_from_wm() const {
  return has_unmap_req_from_wm_;
}

void Client::set_workspace(Workspace* workspace) {
  workspace_ = workspace;
}

void Client::set_mapped(bool mapped) {
  is_mapped_ = mapped;
}

void Client::set_floating(bool floating) {
  is_floating_ = floating;
}

void Client::set_fullscreen(bool fullscreen) {
  is_fullscreen_ = fullscreen;
}

void Client::set_has_unmap_req_from_wm(bool has_unmap_req_from_wm) {
  has_unmap_req_from_wm_ = has_unmap_req_from_wm;
}

void Client::set_attr_cache(const XWindowAttributes& attr) {
  attr_cache_ = attr;
}

void Client::RoundCorner(unsigned int radius) const {
  XWindowAttributes attr = GetXWindowAttributes();
  int width = attr.width + attr.border_width;
  int height = attr.height + attr.border_width;
  Pixmap mask = XCreatePixmap(dpy_, window_, width, height, 1);
  XGCValues xgcv;
  GC shape_gc = XCreateGC(dpy_, mask, 0, &xgcv);
  int rad = radius;
  int dia = 2 * rad;

  XSetForeground(dpy_, shape_gc, 0);
  XFillRectangle(dpy_, mask, shape_gc, 0, 0, width, height);
  XSetForeground(dpy_, shape_gc, 1);
  XFillArc(dpy_, mask, shape_gc, 0, 0, dia, dia, 0, 23040);
  XFillArc(dpy_, mask, shape_gc, width-dia-1, 0, dia, dia, 0, 23040);
  XFillArc(dpy_, mask, shape_gc, 0, height-dia-1, dia, dia, 0, 23040);
  XFillArc(dpy_, mask, shape_gc, width-dia-1, height-dia-1, dia, dia, 0, 23040);
  XFillRectangle(dpy_, mask, shape_gc, rad, 0, width-dia, height);
  XFillRectangle(dpy_, mask, shape_gc, 0, rad, width, height-dia);
  XShapeCombineMask(dpy_, window_, ShapeBounding, 0, 0, mask, ShapeSet);
  XFreePixmap(dpy_, mask);
}

void Client::ConstrainSize(int& w, int& h) const {
  const int min_w = (size_hints_.flags & PMinSize) ? size_hints_.min_width : MIN_WINDOW_WIDTH;
  const int min_h = (size_hints_.flags & PMinSize) ? size_hints_.min_height : MIN_WINDOW_HEIGHT;
  w = (w < min_w) ? min_w : w;
  h = (h < min_h) ? min_h : h;
}

Client::Area::Area() : x(), y(), w(), h() {}

Client::Area::Area(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

bool Client::Area::operator==(const Client::Area& other) {
  return (x == other.x) && (y == other.y) && (w == other.w) && (h == other.h);
}

bool Client::Area::operator!=(const Client::Area& other) {
  return (x != other.x) || (y != other.y) || (w != other.w) || (h != other.h);
}

}  // namespace wmderland
