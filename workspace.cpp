#include "workspace.hpp"

Workspace::Workspace(Display* dpy, short id) {
    dpy_ = dpy;
    id_ = id;
}


void Workspace::MapAllWindows() {
    for (auto const w : windows_) {
        XMapWindow(dpy_, w);
    }
}

void Workspace::UnmapAllWindows() {
    for (auto const w : windows_) {
        XUnmapWindow(dpy_, w);
    }
}


void Workspace::Add(const Window w) {
    windows_.push_back(w);
}

void Workspace::Remove(const Window w) {
    windows_.erase(std::remove(windows_.begin(), windows_.end(), w), windows_.end());
}

bool Workspace::Has(const Window w) {
    return std::find(windows_.begin(), windows_.end(), w) != windows_.end();
}


short Workspace::id() {
    return id_;
}

Window Workspace::active_window() {
    return active_window_;
}
