#include "client.hpp"
#include "global.hpp"
#include "util.hpp"

std::unordered_map<Window, Client*> Client::mapper_;

Client::Client(Display* dpy, Window window, Workspace* workspace) {
    dpy_ = dpy;
    window_ = window;
    workspace_ = workspace;
    position_ = {-1, -1};
    is_bar_ = wm_utils::IsBar(wm_class_);
    wm_class_ = wm_utils::QueryWmClass(dpy, window);
    
    mapper_[window_] = this;

    XSelectInput(dpy, window, FocusChangeMask);
    SetBorderWidth(BORDER_WIDTH);
    SetBorderColor(FOCUSED_COLOR);
}

Client::~Client() {
    mapper_.erase(window_);
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

Workspace* Client::workspace() {
    return workspace_;
}

void Client::set_workspace(Workspace* workspace) {
    workspace_ = workspace;
}


bool Client::is_bar() {
    return is_bar_;
}

std::string Client::wm_class() {
    return wm_class_;
}

std::pair<short, short> Client::position() {
    return position_;
}

void Client::set_position(std::pair<short, short> position) {
    position_ = position;
}
