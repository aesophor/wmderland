#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <X11/Xlib.h>

/* A Client is any window that we have decided to manage */
class Client {
public:
    Client(Display* dpy, Window window);
    ~Client();

    void SetFocused(bool is_focused);
    Window window();
    bool is_decorated();
    bool is_floating();
    bool is_focused();
    bool is_mapped();
private:
    Display* dpy_;
    Window window_;
    bool is_decorated_;
    bool is_floating_;
    bool is_focused_;
    bool is_mapped_;
};

#endif
