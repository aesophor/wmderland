#include "properties.hpp"

Properties::Properties(Display* dpy) {
    utf8string = XInternAtom(dpy, "UTF8_STRING", false); 

    wm_atoms[atom::WM_PROTOCOLS] = XInternAtom(dpy, "WM_PROTOCOLS", false);
    wm_atoms[atom::WM_DELETE] = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
    wm_atoms[atom::WM_STATE] = XInternAtom(dpy, "WM_STATE", false);
    wm_atoms[atom::WM_TAKE_FOCUS] = XInternAtom(dpy, "WM_TAKE_FOCUS", false);
    
    net_atoms[atom::NET_SUPPORTED] = XInternAtom(dpy, "_NET_SUPPORTED", false);
    net_atoms[atom::NET_WM_NAME] = XInternAtom(dpy, "_NET_WM_NAME", false);
    net_atoms[atom::NET_WM_STATE] = XInternAtom(dpy, "_NET_WM_STATE", false);
    net_atoms[atom::NET_SUPPORTING_WM_CHECK] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", false);
    net_atoms[atom::NET_WM_STATE_FULLSCREEN] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false);
    net_atoms[atom::NET_ACTIVE_WINDOW] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false);
    net_atoms[atom::NET_WM_WINDOW_TYPE] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false);
    net_atoms[atom::NET_WM_WINDOW_TYPE_DIALOG] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false);
    net_atoms[atom::NET_WM_WINDOW_TYPE_NOTIFICATION] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NOTIFICATION", false);
    net_atoms[atom::NET_CLIENT_LIST] = XInternAtom(dpy, "_NET_CLIENT_LIST", false);
};
