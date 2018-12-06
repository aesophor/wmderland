#include "workspace.hpp"
#include "config.hpp"
#include <algorithm>
#include <string>
#include <glog/logging.h>

using std::pair;
using std::vector;

Workspace::Workspace(Display* dpy, short id) {
    dpy_ = dpy;
    id_ = id;
    has_fullscreen_application_ = false;
    active_client_pos_ = {-1, -1};
}

Workspace::~Workspace() {
    for (auto column : clients_) {
        for (auto client : column) {
            delete client;
        }
    }
}


void Workspace::Add(Window w, Direction tiling_direction, bool is_floating) {
    switch (tiling_direction) {
        case HORIZONTAL:
            AddHorizontal(w, is_floating);
            break;

        case VERTICAL:
            AddVertical(w, is_floating);
            break;

        default:
            break;
    }
}

void Workspace::AddHorizontal(Window w, bool is_floating) {
    Client* c = new Client(dpy_, w, this);
    c->set_floating(is_floating);

    // If active_client_pos_.first (i.e., active client's column) == last column in that row,
    // we push_back() a vector of Client* to create a new column at the end of that row,
    // and then push_back() the window to that newly created column.
    //
    // Otherwise, insert a new column at the next position of active_client_pos_.first,
    // and then push_back() the window to that newly created column.
    short last_col = (short) clients_.size() - 1;

    if (active_client_pos_.first == last_col) {
        clients_.push_back(vector<Client*>());
        clients_[last_col + 1].push_back(c);
    } else {
        clients_.insert(clients_.begin() + active_client_pos_.first + 1, vector<Client*>());
        clients_[active_client_pos_.first + 1].push_back(c);
    }

    active_client_pos_.first++;
    active_client_pos_.second = 0;
}

void Workspace::AddVertical(Window w, bool is_floating) {
    if (clients_.size() == 0) {
        AddHorizontal(w, is_floating);
        return;
    }

    Client* c = new Client(dpy_, w, this);
    c->set_floating(is_floating);

    vector<Client*>& active_client_pos_col = clients_[active_client_pos_.first];
    short last_row = (short) active_client_pos_col.size() - 1;

    if (active_client_pos_.second == last_row) {
        active_client_pos_col.push_back(c);
    } else {
        active_client_pos_col.insert(active_client_pos_col.begin() + active_client_pos_.second + 1, c);
    }

    active_client_pos_.second++;
}


void Workspace::Remove(Window w) {
    vector<Client*>* client_col;
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
    bool is_floating = Client::mapper_[w]->is_floating();
    Remove(w);
    workspace->AddHorizontal(w, is_floating);
}

bool Workspace::Has(Window w) {
    return Get(w) != nullptr;
}

bool Workspace::IsEmpty() {
    return clients_.size() == 0;
}

short Workspace::ColSize() const {
    return clients_.size();
}

short Workspace::RowSize(short col_idx) const {
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

Client* Workspace::GetByIndex(pair<short, short> pos) {
    if (clients_.size() == 0) return nullptr;
    if (pos.first < 0 || pos.second < 0) return nullptr;

    if (pos.first >= (short) clients_.size()) return nullptr;
    if (pos.second >= (short) clients_[pos.first].size()) return nullptr;
    return clients_[pos.first][pos.second];
}

vector<Client*> Workspace::GetFloatingClients() {
    vector<Client*> floating_clients;

    for (size_t col = 0; col < clients_.size(); col++) {
        for (size_t row = 0; row < clients_[col].size(); row++) {
            Client* c = clients_[col][row];
            if (c->is_floating()) {
                floating_clients.push_back(c);
            }
        }
    }

    return floating_clients;
}

vector<vector<Client*> > Workspace::GetTilingClients() {
    vector<vector<Client*> > tiling_clients;

    for (size_t col = 0; col < clients_.size(); col++) {
        vector<Client*> tiling_col;
        
        for (size_t row = 0; row < clients_[col].size(); row++) {
            Client* c = clients_[col][row];
            if (!c->is_floating()) {
                tiling_col.push_back(c);
            }
        }

        if (!tiling_col.empty()) {
            tiling_clients.push_back(tiling_col);
        }
    }
    
    return tiling_clients;
}


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

void Workspace::RaiseAllFloatingClients() {
    vector<Client*> floating_clients = GetFloatingClients();

    for (auto c : floating_clients) {
        UnsetFocusClient();
        SetFocusClient(c->window());
    }
}

void Workspace::SetFocusClient(Window w) {
    Client* c = Client::mapper_[w];

    // Raise the window to the top and set input focus to it.
    if (c) {
        XRaiseWindow(dpy_, w);
        XSetInputFocus(dpy_, w, RevertToParent, CurrentTime);

        c->SetBorderColor(Config::GetInstance()->focused_color());
    }
}

void Workspace::UnsetFocusClient() {
    Client* c = active_client();
    if (c) {
        c->SetBorderColor(Config::GetInstance()->unfocused_color());
    }
}


void Workspace::FocusLeft() {
    if (active_client_pos_.first <= 0) return;

    GetByIndex(active_client_pos_)->SetBorderColor(Config::GetInstance()->unfocused_color());
    active_client_pos_.first--;

    if (active_client_pos_.second >= (short) clients_[active_client_pos_.first].size()) {
        active_client_pos_.second = clients_[active_client_pos_.first].size() - 1;
    }

    SetFocusClient(GetByIndex(active_client_pos_)->window());
}

void Workspace::FocusRight() {
    if (active_client_pos_.first >= (short) clients_.size() - 1) return;

    GetByIndex(active_client_pos_)->SetBorderColor(Config::GetInstance()->unfocused_color());
    active_client_pos_.first++;

    if (active_client_pos_.second >= (short) clients_[active_client_pos_.first].size()) {
        active_client_pos_.second = clients_[active_client_pos_.first].size() - 1;
    }

    SetFocusClient(GetByIndex(active_client_pos_)->window());
}

void Workspace::FocusUp() {
    if (active_client_pos_.second <= 0) return;
    GetByIndex(active_client_pos_)->SetBorderColor(Config::GetInstance()->unfocused_color());
    active_client_pos_.second--;
    SetFocusClient(GetByIndex(active_client_pos_)->window());
}

void Workspace::FocusDown() {
    if (active_client_pos_.second >= (short) clients_[active_client_pos_.first].size() - 1) return;
    GetByIndex(active_client_pos_)->SetBorderColor(Config::GetInstance()->unfocused_color());
    active_client_pos_.second++;
    SetFocusClient(GetByIndex(active_client_pos_)->window());
}


short Workspace::id() {
    return id_;
}

bool Workspace::has_fullscreen_application() {
    return has_fullscreen_application_;
}

void Workspace::set_has_fullscreen_application(bool has_fullscreen_application) {
    has_fullscreen_application_ = has_fullscreen_application;
}

Client* Workspace::active_client() {
    return GetByIndex(active_client_pos_);
}

pair<short, short> Workspace::active_client_pos() {
    return active_client_pos_;
}
