// A Client is any window that we have decided to manage. It is a wrapper class 
// of Window which provides some useful information and methods.

#ifndef WMDERLAND_CLIENT_HPP_
#define WMDERLAND_CLIENT_HPP_

#include "workspace.hpp"
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <unordered_map>
#include <string>

class Workspace;

class Client {
public:
    // The lightning fast mapper which maps Window to Client* in O(1)
    static std::unordered_map<Window, Client*> mapper_;

    Client(Display* dpy, Window window, Workspace* workspace);
    virtual ~Client();
    
    void Map() const;
    void Unmap() const;
    void Raise() const;
    void SetInputFocus() const;
    void SetBorderWidth(unsigned int width) const;
    void SetBorderColor(unsigned long color) const;
    XWindowAttributes GetXWindowAttributes() const;

    Window window() const;
    Workspace* workspace() const;
    XWindowAttributes& previous_attr();

    bool is_floating() const;
    bool is_fullscreen() const;
    void set_workspace(Workspace* workspace);
    void set_previous_attr(const XWindowAttributes& attr);
    void set_floating(bool is_floating);
    void set_fullscreen(bool is_fullscreen);

private:
    Display* dpy_;
    Window window_;
    Workspace* workspace_;
    XWindowAttributes previous_attr_;

    bool is_floating_;
    bool is_fullscreen_;
};

#endif
