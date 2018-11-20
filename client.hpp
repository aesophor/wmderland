#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>

/* A Client is any window that we have decided to manage */
class Client {
public:
    Client(Display* dpy, Window window);
    ~Client();

    void SetBorderWidth(unsigned int width);
    void SetBorderColor(unsigned long color);
    void SetFocused(bool is_focused);

    Window window();
    std::string& wm_class();

private:
    Display* dpy_;
    Window window_;
    std::string wm_class_;

    unsigned int border_width_;
    unsigned long border_color_;
};

#endif
