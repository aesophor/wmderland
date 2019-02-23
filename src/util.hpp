#ifndef WMDERLAND_UTIL_HPP_
#define WMDERLAND_UTIL_HPP_

#include "properties.hpp"
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
}
#include <string>
#include <vector>

namespace tiling {

    enum Direction {
        UNSPECIFIED,
        HORIZONTAL,
        VERTICAL
    };
 
}

struct Area {
    Area();
    Area(int x, int y, int width, int height);

    bool operator==(const Area& other);
    bool operator!=(const Area& other);

    int x, y, width, height;
};

namespace wm_utils {
    std::pair<int, int> GetDisplayResolution(Display* dpy, Window root_window);
    XWindowAttributes GetWindowAttributes(Display* dpy, Window w);
    XSizeHints GetWmNormalHints(Display* dpy, Window w);
    XClassHint GetXClassHint(Display* dpy, Window w);
    std::string GetNetWmName(Display* dpy, Window w, Properties* prop);
    std::string GetWmName(Display* dpy, Window w);
    void SetWindowWmState(Display* dpy, Window w, unsigned long state, Properties* prop);
    void SetNetActiveWindow(Display* dpy, Window root_window, Window w, Properties* prop);
    void ClearNetActiveWindow(Display* dpy, Window root_window, Properties* prop);
    Atom* GetWindowProperty(Display* dpy, Window w, Atom property, unsigned long* atom_len);
    bool WindowPropertyHasAtom(Display* dpy, Window w, Atom property, Atom target_atom);

    std::string KeysymToStr(Display* dpy, unsigned int keycode);
    unsigned int StrToKeycode(Display* dpy, const std::string& key_name);
    std::string KeymaskToStr(int modifier);
    int StrToKeymask(const std::string& modifier_str, bool shift);
} // namespace wm_utils

namespace string_utils {
    std::vector<std::string> Split(const std::string& s, const char delimiter);
    std::vector<std::string> Split(const std::string& s, const char delimiter, int count);
    bool StartsWith(const std::string& s, const std::string& keyword);
    bool Contains(const std::string& s, const std::string& keyword);
    void Replace(std::string& s, const std::string& keyword, const std::string& newword);
    void Strip(std::string& s);
} // namespace string_utils

namespace sys_utils {
    std::string ToAbsPath(const std::string& path);
} // namespace sys_utils

#endif
