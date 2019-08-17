// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_CLIENT_H_
#define WMDERLAND_CLIENT_H_

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <unordered_map>
#include <string>

namespace wmderland {

class Workspace;

// A Client is any window that we have decided to manage. It is a wrapper class 
// of Window which provides some useful information and methods.
class Client {
 public:
  struct Area {
    Area();
    Area(int x, int y, int w, int h);
    bool operator== (const Client::Area& other);
    bool operator!= (const Client::Area& other);

    int x, y, w, h;
  };

  // The lightning fast mapper which maps Window to Client* in O(1)
  static std::unordered_map<Window, Client*> mapper_;

  Client(Display* dpy, Window window, Workspace* workspace);
  virtual ~Client();

  inline void Map() const;
  inline void Unmap();
  inline void Raise() const;
  inline void Move(int x, int y) const;
  inline void Resize(int w, int h) const;
  inline void MoveResize(int x, int y, int w, int h) const;
  inline void MoveResize(int x, int y, const std::pair<int, int>& size) const;
  inline void SetInputFocus() const;
  inline void SetBorderWidth(unsigned int width) const;
  inline void SetBorderColor(unsigned long color) const;
  XWindowAttributes GetXWindowAttributes() const;

  Window window() const;
  Workspace* workspace() const;
  const XSizeHints& size_hints() const;
  const XWindowAttributes& attr_cache() const;
 
  bool is_mapped() const;
  bool is_floating() const;
  bool is_fullscreen() const;
  bool has_unmap_req_from_user() const;

  void set_workspace(Workspace* workspace);
  void set_mapped(bool mapped);
  void set_floating(bool floating);
  void set_fullscreen(bool fullscreen);
  void set_has_unmap_req_from_user(bool has_unmap_req_from_user);
  void set_attr_cache(const XWindowAttributes& attr);

 private:
  Display* dpy_;
  Window window_;
  Workspace* workspace_;
  XSizeHints size_hints_;
  XWindowAttributes attr_cache_;

  bool is_mapped_;
  bool is_floating_;
  bool is_fullscreen_;

  bool has_unmap_req_from_user_;
};


inline void Client::Map() const {
  XMapWindow(dpy_, window_);
}

inline void Client::Unmap() {
  // If this client is already unmapped, or the user has already sent a request
  // to unmap it, then no need to do it again.
  if (!is_mapped_ || has_unmap_req_from_user_) {
    return;
  }

  has_unmap_req_from_user_ = true; // will be set to false in WindowManager::OnUnmapNotify
  XUnmapWindow(dpy_, window_);
}

inline void Client::Raise() const {
  XRaiseWindow(dpy_, window_);
}

inline void Client::Move(int x, int y) const {
  XMoveWindow(dpy_, window_, x, y);
}

inline void Client::Resize(int w, int h) const {
  XResizeWindow(dpy_, window_, w, h);
}

inline void Client::MoveResize(int x, int y, int w, int h) const {
  XMoveResizeWindow(dpy_, window_, x, y, w, h);
}

inline void Client::MoveResize(int x, int y, const std::pair<int, int>& size) const {
  XMoveResizeWindow(dpy_, window_, x, y, size.first, size.second);
}

inline void Client::SetInputFocus() const {
  XSetInputFocus(dpy_, window_, RevertToParent, CurrentTime);
}

inline void Client::SetBorderWidth(unsigned int width) const {
  XSetWindowBorderWidth(dpy_, window_, width);
}

inline void Client::SetBorderColor(unsigned long color) const {
  XSetWindowBorder(dpy_, window_, color);
}

} // namespace wmderland

#endif // WMDERLAND_CLIENT_H_
