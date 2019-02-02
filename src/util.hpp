#ifndef WMDERLAND_UTIL_HPP_
#define WMDERLAND_UTIL_HPP_

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
}
#include <string>
#include <vector>
#include "tiling.hpp"

struct WindowPosSize {
    WindowPosSize();
    WindowPosSize(int x, int y, int width, int height);

    int x, y;
    int width, height;

    bool operator==(const WindowPosSize& other);
    bool operator!=(const WindowPosSize& other);
};

namespace wm_utils {
    std::pair<int, int> GetDisplayResolution(Display* dpy, Window root);
    XWindowAttributes QueryWindowAttributes(Display* dpy, Window w);
    XSizeHints QueryWmNormalHints(Display* dpy, Window w);
    XClassHint QueryWmClass(Display* dpy, Window w);
    std::string QueryWmName(Display* dpy, Window w);

    std::string KeysymToStr(Display* dpy, unsigned int keycode, bool shift);
    unsigned int StrToKeycode(Display* dpy, const std::string& key_name);
    std::string KeymaskToStr(int modifier);
    int StrToKeymask(const std::string& modifier_str, bool shift);
    tiling::Action StrToAction(const std::string& action_str);
    
    bool IsFullScreen(Display* dpy, Window w, Atom* atoms);
    bool IsDialogOrNotification(Display* dpy, Window w, Atom* atoms);
    bool IsBar(Display* dpy, Window w);
}

namespace string_utils {
    std::vector<std::string> Split(const std::string& s, const char delimiter);
    std::vector<std::string> Split(const std::string& s, const char delimiter, int count);
    bool StartsWith(const std::string& s, const std::string& keyword);
    bool Contains(const std::string& s, const std::string& keyword);
    void Replace(std::string& s, const std::string keyword, const std::string newword);
    void Trim(std::string& s);
    std::string ToAbsPath(const std::string& path);
}

#endif
