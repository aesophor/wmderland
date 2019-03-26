// A Client is any window that we have decided to manage. It is a wrapper class 
// of Window which provides some useful information and methods.

#ifndef WMDERLAND_CLIENT_H_
#define WMDERLAND_CLIENT_H_

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <unordered_map>
#include <string>

#include "workspace.h"
#include "properties.h"
#include "util.h"

class Workspace;

class Client {
public:
  // The lightning fast mapper which maps Window to Client* in O(1)
  static std::unordered_map<Window, Client*> mapper_;

  Client(Display* dpy, Window window, Workspace* workspace);
  virtual ~Client();

  inline void Map() const;
  inline void Unmap() const;
  inline void Raise() const;
  inline void Move(int x, int y) const;
  inline void Resize(int w, int h) const;
  inline void MoveResize(int x, int y, int w, int h) const;
  inline void MoveResize(int x, int y, const std::pair<int, int>& size) const;
  inline void SetInputFocus() const;
  inline void SetBorderWidth(unsigned int width) const;
  inline void SetBorderColor(unsigned long color) const;
  void SaveXWindowAttributes();

  Window window() const;
  Workspace* workspace() const;
  const XSizeHints& size_hints() const;
  const XWindowAttributes& previous_attr() const;

  bool is_floating() const;
  bool is_fullscreen() const;
  void set_workspace(Workspace* workspace);
  void set_floating(bool is_floating);
  void set_fullscreen(bool is_fullscreen);

private:
  Display* dpy_;
  Window window_;
  Workspace* workspace_;
  XSizeHints size_hints_;
  XWindowAttributes previous_attr_;

  bool is_floating_;
  bool is_fullscreen_;
};


inline void Client::Map() const {
  XMapWindow(dpy_, window_);
}

inline void Client::Unmap() const {
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

#endif
