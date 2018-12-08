#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include "workspace.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unordered_map>
#include <string>

/* A Client is any window that we have decided to manage.
 * It is a wrapper class of Window which provides some 
 * useful information and methods.
 */ 
class Workspace;

class Client {
public:
    Client(Display* dpy, Window window, Workspace* workspace);
    ~Client();
    static std::unordered_map<Window, Client*> mapper_;

    void SetBorderWidth(unsigned int width);
    void SetBorderColor(unsigned long color);
    XWindowAttributes GetXWindowAttributes();

    Window& window();
    Workspace* workspace();
    XWindowAttributes& previous_attr();
    
    bool is_bar();
    bool is_floating();
    bool is_fullscreen();

    void set_workspace(Workspace* workspace);
    void set_floating(bool is_floating);
    void set_fullscreen(bool is_fullscreen);

private:
    Display* dpy_;
    Window window_;
    Workspace* workspace_;
    XWindowAttributes previous_attr_;

    bool is_bar_;
    bool is_floating_;
    bool is_fullscreen_;
};

#endif
