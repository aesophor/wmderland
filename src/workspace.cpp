#include "workspace.hpp"
#include "global.hpp"
#include <algorithm>
#include <string>

Workspace::Workspace(Display* dpy, short id) {
    dpy_ = dpy;
    id_ = id;
}


void Workspace::Add(Client* c) {
    clients_.push_back(c);
}

void Workspace::Remove(Window w) {
    Client* c = Get(w);
    clients_.erase(std::remove(clients_.begin(), clients_.end(), c), clients_.end());
    delete c;
}

bool Workspace::Has(Window w) {
    return Get(w) != nullptr;
}

Client* Workspace::Get(Window w) {
    for (auto const c : clients_) {
        if (c->window() == w) {
            return c;
        }
    }
    return nullptr;
}

std::string Workspace::ToString() {
    bool has_previous_item = false;
    std::string output = "[";
    for (auto const c : clients_) {
        if (has_previous_item) {
            output += ", ";
        }
        output += c->wm_class();
        if (!has_previous_item) {
            has_previous_item = true;
        }
    }
    return output + "]";
}


void Workspace::MapAllClients() {
    for (auto const c : clients_) {
        XMapWindow(dpy_, c->window());
    }
}

void Workspace::UnmapAllClients() {
    for (auto const c : clients_) {
        XUnmapWindow(dpy_, c->window());
    }
}

void Workspace::SetFocusClient(Window focused_window) {
    // Raise the window to the top and set input focus to it.
    XRaiseWindow(dpy_, focused_window);
    XSetInputFocus(dpy_, focused_window, RevertToParent, CurrentTime);

    // For all clients (i.e., windows we've decided to manage) in this workspace,
    // set all of their border colors to UNFOCUSED_COLOR except focused_window.
    for (auto const c : clients_) {
        c->SetBorderColor((c->window() == focused_window) ? FOCUSED_COLOR : UNFOCUSED_COLOR);
        if (c->window() == focused_window) active_client_ = c;
    }
}

void Workspace::Center(Window w) {
    Client* c = Get(w);
    
}


short Workspace::id() {
    return id_;
}

Client* Workspace::active_client() {
    return active_client_;
}
