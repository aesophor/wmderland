#include "properties.hpp"

Properties::Properties(Display* dpy) : utf8string(XInternAtom(dpy, "UTF8_STRING", false)) {
    wm[atom::WM_PROTOCOLS] = XInternAtom(dpy, "WM_PROTOCOLS", false);
    wm[atom::WM_DELETE] = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
    wm[atom::WM_STATE] = XInternAtom(dpy, "WM_STATE", false);
    wm[atom::WM_TAKE_FOCUS] = XInternAtom(dpy, "WM_TAKE_FOCUS", false);
    
    net[atom::NET_SUPPORTED] = XInternAtom(dpy, "_NET_SUPPORTED", false);
    net[atom::NET_SUPPORTING_WM_CHECK] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", false);
    net[atom::NET_ACTIVE_WINDOW] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", false);
    net[atom::NET_NUMBER_OF_DESKTOPS] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", false);
    net[atom::NET_CURRENT_DESKTOP] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", false);
    net[atom::NET_DESKTOP_VIEWPORT] = XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", false);
    net[atom::NET_DESKTOP_NAMES] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", false);
    net[atom::NET_WM_NAME] = XInternAtom(dpy, "_NET_WM_NAME", false);
    net[atom::NET_WM_STATE] = XInternAtom(dpy, "_NET_WM_STATE", false);
    net[atom::NET_WM_STATE_FULLSCREEN] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", false);
    net[atom::NET_WM_WINDOW_TYPE] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", false);
    net[atom::NET_WM_WINDOW_TYPE_DOCK] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", false);
    net[atom::NET_WM_WINDOW_TYPE_DIALOG] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", false);
    net[atom::NET_WM_WINDOW_TYPE_SPLASH] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_SPLASH", false);
    net[atom::NET_WM_WINDOW_TYPE_NOTIFICATION] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NOTIFICATION", false);
    net[atom::NET_CLIENT_LIST] = XInternAtom(dpy, "_NET_CLIENT_LIST", false);
};
