#include "client.hpp"
#include "config.hpp"
#include "util.hpp"

using std::string;

std::unordered_map<Window, Client*> Client::mapper_;

Client::Client(Display* dpy, Window window, Workspace* workspace) {
    dpy_ = dpy;
    window_ = window;
    workspace_ = workspace;

    is_bar_ = wm_utils::IsBar(dpy, window);
    is_floating_ = false;
    is_fullscreen_ = false;
    
    mapper_[window_] = this;

    SetBorderWidth(Config::GetInstance()->border_width());
    SetBorderColor(Config::GetInstance()->focused_color());
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

XWindowAttributes& Client::previous_attr() {
    return previous_attr_;
}


bool Client::is_bar() {
    return is_bar_;
}

bool Client::is_floating() {
    return is_floating_;
}

bool Client::is_fullscreen() {
    return is_fullscreen_;
}


void Client::set_workspace(Workspace* workspace) {
    workspace_ = workspace;
}

void Client::set_floating(bool is_floating) {
    is_floating_ = is_floating;
}

void Client::set_fullscreen(bool is_fullscreen) {
    is_fullscreen_ = is_fullscreen;
}
