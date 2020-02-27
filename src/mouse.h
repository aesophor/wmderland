// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_MOUSE_H_
#define WMDERLAND_MOUSE_H_

extern "C" {
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
}

namespace wmderland {

class Mouse {
 public:
  Mouse(Display* dpy, Window root_window);
  virtual ~Mouse() = default;

  enum Button {
    LEFT = 1,
    MID,
    RIGHT,
  };

  enum CursorType {
    NORMAL,
    MOVE,
    RESIZE = 3,
  };

  void SetCursor(Mouse::CursorType type) const;

 private:
  Display* dpy_;
  Window root_window_;

  Cursor cursors_[4];
  XButtonEvent btn_pressed_event_;  // window move/resize event cache

  friend class WindowManager;
};

}  // namespace wmderland

#endif  // WMDERLAND_MOUSE_H_
