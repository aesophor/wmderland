// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_CLIENT_H_
#define WMDERLAND_CLIENT_H_

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <string>
#include <unordered_map>
#include "action.h"

namespace wmderland {

class Workspace;

// A Client is any window that we have decided to manage. It is a wrapper class
// of Window which provides some useful information and methods.
class Client {
 public:
  struct Area {
    Area();
    Area(int x, int y, int w, int h);
    bool operator==(const Client::Area& other);
    bool operator!=(const Client::Area& other);

    int x, y, w, h;
  };

  // The lightning fast mapper which maps Window to Client* in O(1)
  static std::unordered_map<Window, Client*> mapper_;

  Client(Display* dpy, Window window, Workspace* workspace);
  virtual ~Client();

  void Map() const;
  void Unmap();
  void Raise() const;
  void Move(int x, int y, bool absolute = true) const;
  void Resize(int w, int h, bool absolute = true) const;
  void MoveResize(int x, int y, int w, int h, bool absolute = true) const;
  void MoveResize(int x, int y, const std::pair<int, int>& size) const;
  void SetInputFocus() const;
  void SetBorderWidth(unsigned int width) const;
  void SetBorderColor(unsigned long color) const;
  void SelectInput(long input_mask) const;
  XWindowAttributes GetXWindowAttributes() const;

  void Move(const Action& action) const;  // FLOAT_MOVE_{LEFT,RIGHT,UP,DOWN}
  void Resize(const Action& action) const;  // FLOAT_RESIZE_{LEFT,RIGHT,UP,DOWN}

  Window window() const;
  Workspace* workspace() const;
  const XSizeHints& size_hints() const;
  const XWindowAttributes& attr_cache() const;

  bool is_mapped() const;
  bool is_floating() const;
  bool is_fullscreen() const;
  bool has_unmap_req_from_wm() const;

  void set_workspace(Workspace* workspace);
  void set_mapped(bool mapped);
  void set_floating(bool floating);
  void set_fullscreen(bool fullscreen);
  void set_has_unmap_req_from_wm(bool has_unmap_req_from_user);
  void set_attr_cache(const XWindowAttributes& attr);

 private:
  void RoundCorner(unsigned int radius) const;
  void ConstrainSize(int& w, int& h) const;

  Display* dpy_;
  Window window_;
  Workspace* workspace_;
  XSizeHints size_hints_;
  XWindowAttributes attr_cache_;

  bool is_mapped_;
  bool is_floating_;
  bool is_fullscreen_;

  bool has_unmap_req_from_wm_;
};

}  // namespace wmderland

#endif  // WMDERLAND_CLIENT_H_
