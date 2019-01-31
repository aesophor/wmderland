#ifndef WMDERLAND_CLIENT_HPP_
#define WMDERLAND_CLIENT_HPP_

extern "C" {
#include <X11/Xlib.h>
}

class Client {
public:
    Client(Display* dpy, Window w);
    virtual ~Client();

    void SetBorderWidth(unsigned int width);
    void SetBorderColor(unsigned long color);

    Window window();

private:
    Display* dpy_;
    Window window_;
};

#endif
