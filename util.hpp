#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>

namespace wm_utils {
    std::string QueryWmClass(Display* dpy, Window w);
    bool IsBar(const std::string& wm_class);
    bool IsBar(Display* dpy, Window w);
}

#endif
