#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>

/* A Client is any window that we have decided to manage.
 * It is a wrapper class of Window which provides some 
 * useful information and methods.
 */
class Client {
public:
    Client(Display* dpy, Window window);
    ~Client();

    void SetBorderWidth(unsigned int width);
    void SetBorderColor(unsigned long color);
    XWindowAttributes GetXWindowAttributes();

    Window& window();
    std::string& wm_class();
    bool is_bar();

private:
    Display* dpy_;
    Window window_;
    std::string wm_class_;
    bool is_bar_;

    Client* left_;
    Client* right_;
    Client* up_;
    Client* down_;
};

#endif
