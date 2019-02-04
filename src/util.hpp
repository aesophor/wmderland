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

struct Area {
    Area();
    Area(int x, int y, int width, int height);
    bool operator==(const Area& other);
    bool operator!=(const Area& other);

    int x, y, width, height;
};

namespace tiling {

    enum Direction {
        UNSPECIFIED,
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
        EXIT,
        EXEC,
        UNDEFINED
    };

}

namespace wm_utils {
    std::pair<int, int> GetDisplayResolution(Display* dpy, Window root_window);
    XWindowAttributes GetWindowAttributes(Display* dpy, Window w);
    XSizeHints GetWmNormalHints(Display* dpy, Window w);
    XClassHint GetWmClass(Display* dpy, Window w);
    std::string GetWmName(Display* dpy, Window w);
    Atom* GetPropertyAtoms(Display* dpy, Window w, Atom property, unsigned long* atom_len);
    bool WindowPropertyHasAtom(Display* dpy, Window w, Atom property, Atom target_atom);

    std::string KeysymToStr(Display* dpy, unsigned int keycode, bool shift);
    unsigned int StrToKeycode(Display* dpy, const std::string& key_name);
    std::string KeymaskToStr(int modifier);
    int StrToKeymask(const std::string& modifier_str, bool shift);
    tiling::Action StrToAction(const std::string& action_str);
}

namespace string_utils {
    std::vector<std::string> Split(const std::string& s, const char delimiter);
    std::vector<std::string> Split(const std::string& s, const char delimiter, int count);
    bool StartsWith(const std::string& s, const std::string& keyword);
    bool Contains(const std::string& s, const std::string& keyword);
    void Replace(std::string& s, const std::string& keyword, const std::string& newword);
    void Strip(std::string& s);
}

namespace sys_utils {
    std::string ToAbsPath(const std::string& path);
}

#endif
