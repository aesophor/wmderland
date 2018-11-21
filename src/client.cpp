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
}

Client::~Client() {

}


void Client::SetBorderWidth(unsigned int width) {
    XSetWindowBorderWidth(dpy_, window_, width);
}

void Client::SetBorderColor(unsigned long color) {
    XSetWindowBorder(dpy_, window_, color);
}

XWindowAttributes Client::GetXWindowAttributes() {
    return wm_utils::QueryWindowAttributes(dpy_, window_);
}


Window& Client::window() {
    return window_;
}

std::string& Client::wm_class() {
    return wm_class_;
}

bool Client::is_bar() {
    return is_bar_;
}
