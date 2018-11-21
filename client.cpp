#include "client.hpp"
#include "global.hpp"
#include "util.hpp"

Client::Client(Display* dpy, Window window) {
    dpy_ = dpy;
    window_ = window;
    wm_class_ = wm_utils::QueryWmClass(dpy, window);
    is_bar_ = wm_utils::IsBar(wm_class_);

    XSelectInput(dpy, window, FocusChangeMask);
    SetBorderWidth(BORDER_WIDTH);
    SetBorderColor(FOCUSED_COLOR);
    SetFocused();
}

Client::~Client() {

}


void Client::SetBorderWidth(unsigned int width) {
    XSetWindowBorderWidth(dpy_, window_, width);
}

void Client::SetBorderColor(unsigned long color) {
    XSetWindowBorder(dpy_, window_, color);
}

void Client::SetFocused() {
    // Raise the window to the top and set input focus.
    XRaiseWindow(dpy_, window_);
    XSetInputFocus(dpy_, window_, RevertToParent, CurrentTime);

    // Set the border color to focused.
    SetBorderColor(FOCUSED_COLOR);
}

Window Client::window() {
    return window_;
}

std::string& Client::wm_class() {
    return wm_class_;
}

bool Client::is_bar() {
    return is_bar_;
}
