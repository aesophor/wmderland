#include "client.h"
#include "config.h"

using std::string;

std::unordered_map<Window, Client*> Client::mapper_;

Client::Client(Display* dpy, Window w, Workspace* workspace)
    : dpy_(dpy), window_(w), workspace_(workspace), is_floating_(false), is_fullscreen_(false) {
    mapper_[window_] = this;
    SetBorderWidth(workspace->config()->border_width());
    SetBorderColor(workspace->config()->focused_color());
    XSelectInput(dpy_, window_, StructureNotifyMask | PropertyChangeMask);
}

Client::~Client() {
    mapper_.erase(window_);
}


void Client::SaveXWindowAttributes() {
    previous_attr_ = wm_utils::GetXWindowAttributes(window_);
}


Window Client::window() const {
    return window_;
}

Workspace* Client::workspace() const {
    return workspace_;
}

const XWindowAttributes& Client::previous_attr() const {
    return previous_attr_;
}


bool Client::is_floating() const {
    return is_floating_;
}

bool Client::is_fullscreen() const {
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
