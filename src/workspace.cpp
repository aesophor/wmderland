#include "workspace.hpp"
#include "global.hpp"
#include <algorithm>
#include <string>

Workspace::Workspace(Display* dpy, short id) {
    dpy_ = dpy;
    id_ = id;
    active_client_ = -1;
}

Workspace::~Workspace() {
    for (auto c : clients_) {
        delete c;
    }
}


void Workspace::Add(Window w) {
    Client* c = new Client(dpy_, w);

    // If active_client_ is the last item in the vector, use push_back().
    // Otherwise, insert the new item at the next position of active_client_.
    if (active_client_ == (short) clients_.size() - 1) {
        clients_.push_back(c);
    } else {
        clients_.insert(clients_.begin() + active_client_ + 1, c);
    }

    active_client_++;
}

void Workspace::Remove(Window w) {
    Client* c = Get(w);
    clients_.erase(std::remove(clients_.begin(), clients_.end(), c), clients_.end());
    delete c;

    if (active_client_ >= (short) clients_.size()) {
        active_client_--;
    } else if (clients_.size() == 0) {
        active_client_ = -1;
    }
}

void Workspace::Move(Window w, Workspace* workspace) {
    Client* c = Get(w);
    clients_.erase(std::remove(clients_.begin(), clients_.end(), c), clients_.end());
    workspace->clients_.push_back(c);
}

bool Workspace::Has(Window w) {
    return Get(w) != nullptr;
}

bool Workspace::IsEmpty() {
    return clients_.size() == 0;
}

short Workspace::Size() {
    return clients_.size();
}

Client* Workspace::Get(Window w) {
    for (auto c : clients_) {
        if (c->window() == w) {
            return c;
        }
    }
    return nullptr;
}

Client* Workspace::GetByIndex(short index) {
    return clients_[index];
}

std::string Workspace::ToString() {
    bool has_previous_item = false;
    std::string output = "[";
    for (auto c : clients_) {
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
    for (auto c : clients_) {
        XMapWindow(dpy_, c->window());
    }
}

void Workspace::UnmapAllClients() {
    for (auto c : clients_) {
        XUnmapWindow(dpy_, c->window());
    }
}

void Workspace::SetFocusClient(Window focused_window) {
    // Raise the window to the top and set input focus to it.
    XRaiseWindow(dpy_, focused_window);
    XSetInputFocus(dpy_, focused_window, RevertToParent, CurrentTime);

    // For all clients (i.e., windows we've decided to manage) in this workspace,
    // set all of their border colors to UNFOCUSED_COLOR except focused_window.
    for (size_t i = 0; i < clients_.size(); i++) {
        Client* c = clients_[i];
        c->SetBorderColor((c->window() == focused_window) ? FOCUSED_COLOR : UNFOCUSED_COLOR);

        if (c->window() == focused_window) {
            active_client_ = i;
        }
    }
}


short Workspace::id() {
    return id_;
}

short Workspace::active_client() {
    return active_client_;
}
