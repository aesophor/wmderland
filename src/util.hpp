#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include <vector>

enum Direction {
    HORIZONTAL,
    VERTICAL
};

namespace wm_utils {
    XWindowAttributes QueryWindowAttributes(Display* dpy, Window w);
    std::string QueryWmClass(Display* dpy, Window w);
    std::string QueryWmName(Display* dpy, Window w);
    
    bool IsDialogOrNotification(Display* dpy, Window w, Atom* atoms);
    bool IsBar(const std::string& wm_class);
    bool IsBar(Display* dpy, Window w);
}

namespace string_utils {
    std::vector<std::string> split(const std::string& s, const char delimiter);
    std::vector<std::string> split(const std::string& s, const char delimiter, short count);
    void trim(std::string& s);
}

#endif
