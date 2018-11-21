#ifndef PROPERTY_MANAGER_HPP_
#define PROPERTY_MANAGER_HPP_

#include <X11/Xlib.h>
#include <X11/Xatom.h>

class PropertyManager {
public:
    PropertyManager(Display* dpy);

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
    
    void Set(Window w, Atom property, Atom type, int format, int mode, unsigned char* data, int n_elements);
    void Delete(Window w, Atom property);
    Atom GetWmAtom(short index) const;
    Atom GetNetAtom(short index) const;
    Atom utf8string() const;
    
    friend class WindowManager;

private:
    Display* dpy_;
    Atom utf8string_;
    Atom wm_atoms_[WM_ATOM_SIZE];
    Atom net_atoms_[NET_ATOM_SIZE];
};

#endif
