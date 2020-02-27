// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#include "mouse.h"

namespace wmderland {

Mouse::Mouse(Display* dpy, Window root_window) : dpy_(dpy), root_window_(root_window) {
  cursors_[CursorType::NORMAL] = XCreateFontCursor(dpy, XC_left_ptr);
  cursors_[CursorType::MOVE] = XCreateFontCursor(dpy, XC_fleur);
  cursors_[CursorType::RESIZE] = XCreateFontCursor(dpy, XC_sizing);
}


void Mouse::SetCursor(Mouse::CursorType type) const {
  XDefineCursor(dpy_, root_window_, cursors_[type]);
}

}  // namespace wmderland
