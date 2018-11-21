#include "properties.hpp"

Properties::Properties(Display* dpy) {
    dpy_ = dpy;

    utf8string_ = XInternAtom(dpy_, "UTF8_STRING", false); 

    wm_atoms_[WM_PROTOCOLS] = XInternAtom(dpy_, "WM_PROTOCOLS", false);
    wm_atoms_[WM_DELETE] = XInternAtom(dpy_, "WM_DELETE_WINDOW", false);
    wm_atoms_[WM_STATE] = XInternAtom(dpy_, "WM_STATE", false);
    wm_atoms_[WM_TAKE_FOCUS] = XInternAtom(dpy_, "WM_TAKE_FOCUS", false);
    
    net_atoms_[NET_SUPPORTED] = XInternAtom(dpy_, "_NET_SUPPORTED", false);
    net_atoms_[NET_WM_NAME] = XInternAtom(dpy_, "_NET_WM_NAME", false);
    net_atoms_[NET_WM_STATE] = XInternAtom(dpy_, "_NET_WM_STATE", false);
    net_atoms_[NET_SUPPORTING_WM_CHECK] = XInternAtom(dpy_, "_NET_SUPPORTING_WM_CHECK", false);
    net_atoms_[NET_WM_STATE_FULLSCREEN] = XInternAtom(dpy_, "_NET_WM_STATE_FULLSCREEN", false);
    net_atoms_[NET_ACTIVE_WINDOW] = XInternAtom(dpy_, "_NET_ACTIVE_WINDOW", false);
    net_atoms_[NET_WM_WINDOW_TYPE] = XInternAtom(dpy_, "_NET_WM_WINDOW_TYPE", false);
    net_atoms_[NET_WM_WINDOW_TYPE_DIALOG] = XInternAtom(dpy_, "_NET_WM_WINDOW_TYPE_DIALOG", false);
    net_atoms_[NET_WM_WINDOW_TYPE_NOTIFICATION] = XInternAtom(dpy_, "_NET_WM_WINDOW_TYPE_NOTIFICATION", false);
    net_atoms_[NET_CLIENT_LIST] = XInternAtom(dpy_, "_NET_CLIENT_LIST", false);
};

void Properties::Set(Window w, Atom property, Atom type,
        int format, int mode, unsigned char* data, int n_elements) {
    XChangeProperty(dpy_, w, property, type, format, mode, data, n_elements);
}

void Properties::Delete(Window w, Atom property) {
    XDeleteProperty(dpy_, w, property);
}

Atom Properties::utf8string() const {
    return utf8string_;
}

Atom Properties::GetWmAtom(short index) const {
    return wm_atoms_[index];
}

Atom Properties::GetNetAtom(short index) const {
    return net_atoms_[index];
}
