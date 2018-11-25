#include "workspace.hpp"
#include "global.hpp"
#include <algorithm>
#include <string>
#include <glog/logging.h>

Workspace::Workspace(Display* dpy, short id) {
    dpy_ = dpy;
    id_ = id;
    active_client_ = {-1, -1};
}

Workspace::~Workspace() {
    for (auto column : clients_) {
        for (auto client : column) {
            delete client;
        }
    }
}


void Workspace::AddHorizontal(Window w) {
    Client* c = new Client(dpy_, w);

    // If active_client_.first (i.e., active client's column) == last column in that row,
    // we push_back() a vector of Client* to create a new column at the end of that row,
    // and then push_back() the window to that newly created column.
    //
    // Otherwise, insert a new column at the next position of active_client_.first,
    // and then push_back() the window to that newly created column.
    short last_col = (short) clients_.size() - 1;

    if (active_client_.first == last_col) {
        clients_.push_back(std::vector<Client*>());
        clients_[last_col + 1].push_back(c);
    } else {
        clients_.insert(clients_.begin() + active_client_.first + 1, std::vector<Client*>());
        clients_[active_client_.first + 1].push_back(c);
    }

    active_client_.first++;
    active_client_.second = 0;
}

void Workspace::AddVertical(Window w) {
    if (clients_.size() == 0) {
        AddHorizontal(w);
        return;
    }

    Client* c = new Client(dpy_, w);

    std::vector<Client*>& active_client_col = clients_[active_client_.first];
    short last_row = (short) active_client_col.size() - 1;

    if (active_client_.second == last_row) {
        active_client_col.push_back(c);
    } else {
        active_client_col.insert(active_client_col.begin() + active_client_.second + 1, c);
    }

    active_client_.second++;
}


void Workspace::Remove(Window w) {
    std::vector<Client*>* client_col;
    Client* c;

    for (short col = 0; col < ColSize(); col++) {
        short row_count = RowSize(col);
        for (short row = 0; row < row_count; row++) {
            if (clients_[col][row]->window() == w) {
                client_col = &(clients_[col]);
                c = clients_[col][row];
            }
        }
    }

    if (c == nullptr) return;

    // If that column contains only one client, wipe out that column and delete the client.
    // Otherwise, simply remove the client from that column.
    if ((*client_col).size() == 1) {
        clients_.erase(std::remove(clients_.begin(), clients_.end(), *client_col), clients_.end());
    } else {
        (*client_col).erase(std::remove((*client_col).begin(), (*client_col).end(), c), (*client_col).end());
    }

    delete c;

    // If active_client_'s column is out of range, set it to the last column.
    if (active_client_.first >= (short) clients_.size()) {
        active_client_.first = (short) clients_.size() - 1;
    }

    // If active_client_'s row is out of range, set it to the last row in current column.
    if (active_client_.second >= (short) clients_[active_client_.first].size()) {
        active_client_.second = (short) clients_[active_client_.first].size() - 1;
    } 
}

/*
void Workspace::Move(Window w, Workspace* workspace) {
    Client* c = Get(w);
    clients_.erase(std::remove(clients_.begin(), clients_.end(), c), clients_.end());
    //workspace->clients_.push_back(c);
}
*/

bool Workspace::Has(Window w) {
    return Get(w) != nullptr;
}

bool Workspace::IsEmpty() {
    return clients_.size() == 0;
}

short Workspace::ColSize() {
    return clients_.size();
}

short Workspace::RowSize(short col_idx) {
    return clients_[col_idx].size();
}


Client* Workspace::Get(Window w) {
    for (short col = 0; col < ColSize(); col++) {
        short row_count = RowSize(col);
        for (short row = 0; row < row_count; row++) {
            if (clients_[col][row]->window() == w) {
                return clients_[col][row];
            }
        }
    }
    return nullptr;
}

Client* Workspace::GetByIndex(std::pair<short, short> pos) {
    if (pos.first < 0 || pos.second < 0) return nullptr;

    if (pos.first >= (short) clients_.size()) return nullptr;
    if (pos.second >= (short) clients_[pos.first].size()) return nullptr;
    return clients_[pos.first][pos.second];
}

/*
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
*/


void Workspace::MapAllClients() {
    for (auto column : clients_) {
        for (auto client : column) {
            XMapWindow(dpy_, client->window());
        }
    }
}

void Workspace::UnmapAllClients() {
    for (auto column : clients_) {
        for (auto client : column) {
            XUnmapWindow(dpy_, client->window());
        }
    }
}

void Workspace::SetFocusClient(Window focused_window) {
    // Raise the window to the top and set input focus to it.
    XRaiseWindow(dpy_, focused_window);
    XSetInputFocus(dpy_, focused_window, RevertToParent, CurrentTime);

    // For all clients (i.e., windows we've decided to manage) in this workspace,
    // set all of their border colors to UNFOCUSED_COLOR except focused_window.
    for (size_t col = 0; col < clients_.size(); col++) {
        for (size_t row = 0; row < clients_[col].size(); row++) {
            Client* c = clients_[col][row];
            c->SetBorderColor((c->window() == focused_window) ? FOCUSED_COLOR : UNFOCUSED_COLOR);

            if (c->window() == focused_window) {
                active_client_ = {col, row};
            }
        }
    }
}

void Workspace::FocusLeft() {
    if (active_client_.first <= 0) return;
    active_client_.first--;

    if (active_client_.second >= (short) clients_[active_client_.first].size()) {
        active_client_.second = clients_[active_client_.first].size() - 1;
    }
    SetFocusClient(GetByIndex(active_client_)->window());
}

void Workspace::FocusRight() {
    if (active_client_.first >= (short) clients_.size() - 1) return;
    active_client_.first++;

    if (active_client_.second >= (short) clients_[active_client_.first].size()) {
        active_client_.second = clients_[active_client_.first].size() - 1;
    }
    SetFocusClient(GetByIndex(active_client_)->window());
}

void Workspace::FocusUp() {
    if (active_client_.second <= 0) return;
    active_client_.second--;
    SetFocusClient(GetByIndex(active_client_)->window());
}

void Workspace::FocusDown() {
    if (active_client_.second >= (short) clients_[active_client_.first].size() - 1) return;
    active_client_.second++;
    SetFocusClient(GetByIndex(active_client_)->window());
}


short Workspace::id() {
    return id_;
}

std::pair<short, short> Workspace::active_client() {
    return active_client_;
}
