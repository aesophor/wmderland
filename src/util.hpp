#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <string>
#include <vector>

enum Direction {
    HORIZONTAL,
    VERTICAL
};

enum Action {
    TILE_H,
    TILE_V,
    FOCUS_LEFT,
    FOCUS_RIGHT,
    FOCUS_DOWN,
    FOCUS_UP,
    TOGGLE_FLOATING,
    TOGGLE_FULLSCREEN,
    KILL,
    EXEC,
    UNDEFINED
};

namespace wm_utils {
    std::pair<short, short> GetDisplayResolution(Display* dpy, Window root);
    XWindowAttributes QueryWindowAttributes(Display* dpy, Window w);
    XSizeHints QueryWmNormalHints(Display* dpy, Window w);
    std::string QueryWmClass(Display* dpy, Window w);
    std::string QueryWmName(Display* dpy, Window w);

    unsigned int QueryKeycode(Display* dpy, const std::string& key_name);
    std::string QueryKeysym(Display* dpy, unsigned int keycode, bool shift);
    Action StrToAction(const std::string& action_str);
    
    bool IsDialogOrNotification(Display* dpy, Window w, Atom* atoms);
    bool IsBar(const std::string& wm_class);
    bool IsBar(Display* dpy, Window w);
}

namespace string_utils {
    std::vector<std::string> split(const std::string& s, const char delimiter);
    std::vector<std::string> split(const std::string& s, const char delimiter, short count);
    bool starts_with(const std::string& s, const std::string& keyword);
    bool Contains(const std::string& s, const std::string& keyword);
    void trim(std::string& s);
}

#endif
