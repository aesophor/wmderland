#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include "workspace.hpp"

/* A Client is any window that we have decided to manage.
 * It is a wrapper class of Window which provides some 
 * useful information and methods.
 */
class Workspace;

class Client {
public:
    Client(Display* dpy, Window window, Workspace* workspace);
    ~Client();

    void SetBorderWidth(unsigned int width);
    void SetBorderColor(unsigned long color);
    XWindowAttributes GetXWindowAttributes();

    Window& window();
    Workspace* workspace();
    void set_workspace(Workspace* workspace);

    std::string& wm_class();
    bool is_bar();

private:
    Display* dpy_;
    Window window_;
    Workspace* workspace_;

    std::string wm_class_;
    bool is_bar_;
};

#endif
