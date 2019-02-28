#ifndef WMDERLAND_PROPERTIES_HPP_
#define WMDERLAND_PROPERTIES_HPP_

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xatom.h>
}

namespace atom {

// Default atoms, defined by X
enum {
    WM_PROTOCOLS,
    WM_DELETE,
    WM_STATE,
    WM_TAKE_FOCUS,
    WM_ATOM_SIZE
};

// Extended Window Manager Hints (EWMH) atoms, defined by XDG/freedesktop.org
enum {
    NET_SUPPORTED,
    NET_SUPPORTING_WM_CHECK,
    NET_ACTIVE_WINDOW,
    NET_NUMBER_OF_DESKTOPS,
    NET_CURRENT_DESKTOP,
    NET_DESKTOP_VIEWPORT,
    NET_DESKTOP_NAMES,
    NET_WM_NAME,
    NET_WM_STATE,
    NET_WM_STATE_FULLSCREEN,
    NET_WM_WINDOW_TYPE,
    NET_WM_WINDOW_TYPE_DOCK,
    NET_WM_WINDOW_TYPE_DIALOG,
    NET_WM_WINDOW_TYPE_SPLASH,
    NET_WM_WINDOW_TYPE_NOTIFICATION,
    NET_CLIENT_LIST,
    NET_ATOM_SIZE
};

} // namespace atom

struct Properties {
    Properties(Display* dpy);
    Atom utf8string;
    Atom wm[atom::WM_ATOM_SIZE];
    Atom net[atom::NET_ATOM_SIZE];
};

#endif
