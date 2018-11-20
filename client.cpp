#include "client.hpp"
#include "global.hpp"

Client::Client(Display* dpy, Window window) {
    dpy_ = dpy;
    window_ = window;
}

Client::~Client() {

}

void Client::SetFocused(bool is_focused) {
    // Raise the window to the top and set input focus.
    if (is_focused) {
        XRaiseWindow(dpy_, window_);
        XSetInputFocus(dpy_, window_, RevertToParent, CurrentTime);
    }

    // Set the border color to focused.
    XSetWindowBorder(dpy_, window_, (is_focused) ? FOCUSED_COLOR : UNFOCUSED_COLOR);

    // Set the flag.
    is_focused_ = is_focused;
}

Window Client::window() {
    return window_;
}

bool Client::is_decorated() {
    return is_decorated_;
}
