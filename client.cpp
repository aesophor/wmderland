#include "client.hpp"
#include "global.hpp"

Client::Client(Display* dpy, Window window) {
    dpy_ = dpy;
    window_ = window;
    
    XClassHint hint;
    XGetClassHint(dpy_, window_, &hint);
    wm_class_ = hint.res_class;

    if (wm_class_ != "Polybar") {
        SetBorderWidth(BORDER_WIDTH);
        SetBorderColor(FOCUSED_COLOR);
        SetFocused(true);
    }
}

Client::~Client() {

}


void Client::SetBorderWidth(unsigned int width) {
    if (border_width_ == width) {
        return;
    }

    XSetWindowBorderWidth(dpy_, window_, width);
    border_width_ = width;
}

void Client::SetBorderColor(unsigned long color) {
    if (border_color_ == color) {
        return;
    }

    XSetWindowBorder(dpy_, window_, color);
    border_color_ = color;
}

void Client::SetFocused(bool is_focused) {
    // Raise the window to the top and set input focus.
    if (is_focused) {
        XRaiseWindow(dpy_, window_);
        XSetInputFocus(dpy_, window_, RevertToParent, CurrentTime);
    }

    // Set the border color to focused.
    SetBorderColor((is_focused) ? FOCUSED_COLOR : UNFOCUSED_COLOR);

    // Set the flag.
    //is_focused_ = is_focused;
}

Window Client::window() {
    return window_;
}
