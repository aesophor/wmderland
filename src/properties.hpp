#ifndef PROPERTIES_HPP_
#define PROPERTIES_HPP_

#include <X11/Xlib.h>
#include <X11/Xatom.h>

namespace atom {

    /* Default atoms, defined by X */
    enum {
        WM_PROTOCOLS,
        WM_DELETE,
        WM_STATE,
        WM_TAKE_FOCUS,
        WM_ATOM_SIZE
    };

    /* Extended Window Manager Hints (EWMH) atoms, defined by XDG/freedesktop.org */
    enum {
        NET_SUPPORTED,
        NET_WM_NAME,
        NET_WM_STATE,
        NET_SUPPORTING_WM_CHECK,
        NET_WM_STATE_FULLSCREEN,
        NET_ACTIVE_WINDOW,
        NET_WM_WINDOW_TYPE,
        NET_WM_WINDOW_TYPE_DIALOG,
        NET_WM_WINDOW_TYPE_NOTIFICATION,
        NET_CLIENT_LIST,
        NET_ATOM_SIZE
    };

}

struct Properties {
    Properties(Display* dpy);

    Atom utf8string;
    Atom wm_atoms[atom::WM_ATOM_SIZE];
    Atom net_atoms[atom::NET_ATOM_SIZE];

    friend class WindowManager;
};

#endif
