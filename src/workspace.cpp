#include "workspace.hpp"
#include "global.hpp"
#include <algorithm>
#include <string>
#include <glog/logging.h>

Workspace::Workspace(Display* dpy, short id) {
    dpy_ = dpy;
    id_ = id;
    active_client_pos_ = {-1, -1};
}

Workspace::~Workspace() {
    for (auto column : clients_) {
        for (auto client : column) {
            delete client;
        }
    }
}


void Workspace::AddHorizontal(Window w) {
    Client* c = new Client(dpy_, w, this);

    // If active_client_pos_.first (i.e., active client's column) == last column in that row,
    // we push_back() a vector of Client* to create a new column at the end of that row,
    // and then push_back() the window to that newly created column.
    //
    // Otherwise, insert a new column at the next position of active_client_pos_.first,
    // and then push_back() the window to that newly created column.
    short last_col = (short) clients_.size() - 1;

    if (active_client_pos_.first == last_col) {
        clients_.push_back(std::vector<Client*>());
        clients_[last_col + 1].push_back(c);
    } else {
        clients_.insert(clients_.begin() + active_client_pos_.first + 1, std::vector<Client*>());
        clients_[active_client_pos_.first + 1].push_back(c);
    }

    active_client_pos_.first++;
    active_client_pos_.second = 0;
}

void Workspace::AddVertical(Window w) {
    if (clients_.size() == 0) {
        AddHorizontal(w);
        return;
    }

    Client* c = new Client(dpy_, w, this);

    std::vector<Client*>& active_client_pos_col = clients_[active_client_pos_.first];
    short last_row = (short) active_client_pos_col.size() - 1;

    if (active_client_pos_.second == last_row) {
        active_client_pos_col.push_back(c);
    } else {
        active_client_pos_col.insert(active_client_pos_col.begin() + active_client_pos_.second + 1, c);
    }

    active_client_pos_.second++;
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

    // If active_client_pos_'s column is out of range, set it to the last column.
    if (active_client_pos_.first >= (short) clients_.size()) {
        active_client_pos_.first = (short) clients_.size() - 1;
    }

    // If active_client_pos_'s row is out of range, set it to the last row in current column.
    if (active_client_pos_.second >= (short) clients_[active_client_pos_.first].size()) {
        active_client_pos_.second = (short) clients_[active_client_pos_.first].size() - 1;
    } 
}


void Workspace::Move(Window w, Workspace* workspace) {
    Remove(w);
    workspace->AddHorizontal(w);
}

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
    // We'll get the corresponding client using the lightning fast
    // client mapper which has bigO(1), so we don't have to iterate
    // through the two dimensional clients_ vector!
    Client* c = Client::mapper_[w];

    // But we have to check if it belongs to current workspace!
    if (c && c->workspace() == this) {
        return c;
    }
    return nullptr;
}

Client* Workspace::GetByIndex(std::pair<short, short> pos) {
    if (ColSize() == 0) return nullptr;
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

void Workspace::SetFocusClient(Window w) {
    Client* c = Client::mapper_[w];

    // Raise the window to the top and set input focus to it.
    if (c) {
        XRaiseWindow(dpy_, w);
        XSetInputFocus(dpy_, w, RevertToParent, CurrentTime);

        c->SetBorderColor(FOCUSED_COLOR);
        c->set_position(active_client_pos_);
    }
}


void Workspace::FocusLeft() {
    if (active_client_pos_.first <= 0) return;

    GetByIndex(active_client_pos_)->SetBorderColor(UNFOCUSED_COLOR);
    active_client_pos_.first--;

    if (active_client_pos_.second >= (short) clients_[active_client_pos_.first].size()) {
        active_client_pos_.second = clients_[active_client_pos_.first].size() - 1;
    }

    SetFocusClient(GetByIndex(active_client_pos_)->window());
}

void Workspace::FocusRight() {
    if (active_client_pos_.first >= (short) clients_.size() - 1) return;

    GetByIndex(active_client_pos_)->SetBorderColor(UNFOCUSED_COLOR);
    active_client_pos_.first++;

    if (active_client_pos_.second >= (short) clients_[active_client_pos_.first].size()) {
        active_client_pos_.second = clients_[active_client_pos_.first].size() - 1;
    }

    SetFocusClient(GetByIndex(active_client_pos_)->window());
}

void Workspace::FocusUp() {
    if (active_client_pos_.second <= 0) return;
    GetByIndex(active_client_pos_)->SetBorderColor(UNFOCUSED_COLOR);
    active_client_pos_.second--;
    SetFocusClient(GetByIndex(active_client_pos_)->window());
}

void Workspace::FocusDown() {
    if (active_client_pos_.second >= (short) clients_[active_client_pos_.first].size() - 1) return;
    GetByIndex(active_client_pos_)->SetBorderColor(UNFOCUSED_COLOR);
    active_client_pos_.second++;
    SetFocusClient(GetByIndex(active_client_pos_)->window());
}


short Workspace::id() {
    return id_;
}

Client* Workspace::active_client() {
    return GetByIndex(active_client_pos_);
}

std::pair<short, short> Workspace::active_client_pos() {
    return active_client_pos_;
}
