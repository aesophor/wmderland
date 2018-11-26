#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unordered_map>
#include <string>

class Client;

namespace wm_utils {
    XWindowAttributes QueryWindowAttributes(Display* dpy, Window w);
    std::string QueryWmClass(Display* dpy, Window w);
    std::string QueryWmName(Display* dpy, Window w);
    
    bool IsBar(const std::string& wm_class);
    bool IsBar(Display* dpy, Window w);
}

namespace client_mapper {
    extern std::unordered_map<Window, Client*> mapper;
}

#endif
