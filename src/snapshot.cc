// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "snapshot.h"

extern "C"{
#include <unistd.h>
}
#include <fstream>

#include "client.h"
#include "window_manager.h"
#include "util.h"

using std::endl;
using std::array;
using std::string;
using std::ifstream;
using std::ofstream;

namespace wmderland {

const string Snapshot::kNone_ = "none";
const string Snapshot::kBacktrack_ = "b";
const char Snapshot::kLeafPrefix_ = 'w';
const char Snapshot::kInternalPrefix_ = 'i';
const char Snapshot::kDelimiter_ = ' ';

Snapshot::Snapshot(const string& filename)
    : filename_(sys_utils::ToAbsPath(filename)), failed_count_() {}


bool Snapshot::FileExists() const {
  return access(filename_.c_str(), F_OK) != -1;
}

void Snapshot::Load() {
  WindowManager* wm = WindowManager::GetInstance();
  ifstream fin(filename_);

  // 1. Load failed count.
  // If the same error occurs constantly, throw this exception
  // to immediately halt the window manager.
  fin >> failed_count_;
  if (failed_count_ >= 3) {
    throw SnapshotLoadError();
  }

  // 2. Client deserailization.
  int client_count = 0;
  fin >> client_count;

  for (int i = 0; i < client_count; i++) {
    Window window = None;
    int workspace_id = 0;
    bool is_mapped = false;
    bool is_floating = false;
    bool is_fullscreen = false;
    bool has_unmap_req_from_wm = false;

    fin >> window >> workspace_id
      >> is_mapped >> is_floating >> is_fullscreen
      >> has_unmap_req_from_wm;

    // The ownership of these client objects will be claimed during
    // client tree deserialization!!! See Tree::Deserialize() in tree.cc
    Client* client = new Client(wm->dpy_, window, wm->workspaces_[workspace_id].get());
    client->set_mapped(is_mapped);
    client->set_floating(is_floating);
    client->set_fullscreen(is_fullscreen);
    client->set_has_unmap_req_from_wm(has_unmap_req_from_wm);
  }
  

  // 3. Client Tree deserialization will call Client::mapper_[window],
  // so we have to restore all clients before doing this. See step 1.
  for (const auto& workspace : wm->workspaces_) {
    string data;
    fin >> data;
    workspace->Deserialize(data);
  }

  
  // 4. Current workspace deserialization.
  int current_workspace = 0;
  fin >> current_workspace;
  wm->GotoWorkspace(current_workspace);


  // 5. Docks/Notifications deserialization.
  string line;

  std::getline(fin, line);
  if (line != Snapshot::kNone_) {
    for (const auto& token : string_utils::Split(line, ',')) {
      wm->docks_.push_back(static_cast<Window>(std::stoul(token)));
    }
  }
 
  std::getline(fin, line);
  if (line != Snapshot::kNone_) {
    for (const auto& token : string_utils::Split(line, ',')) {
      wm->notifications_.push_back(static_cast<Window>(std::stoul(token)));
    }
  }


  // 5. Rename snapshot file so that we know we have successfully load it.
  // If the file cannot be renamed and cannot be remove, then throw
  // SnapshotLoadError and the WM will return EXIT_FAILURE.
  if (rename(filename_.c_str(), (filename_ + ".old").c_str()) == -1 &&
      remove(filename_.c_str())) { // remove() returns non-zero on failure
    throw SnapshotLoadError();
  }

  wm->ArrangeWindows();
}

void Snapshot::Save() {
  WindowManager* wm = WindowManager::GetInstance();
  ofstream fout(filename_);

  // 1. Write failed count.
  fout << ++failed_count_ << endl;


  // 2. Client::mapper_ serialization.
  fout << Client::mapper_.size() << endl;

  for (const auto& win_client_pair : Client::mapper_) {
    Window window = win_client_pair.first;
    Client* client = win_client_pair.second;

    fout << window << Snapshot::kDelimiter_
      << client->workspace()->id() << Snapshot::kDelimiter_
      << client->is_mapped() << Snapshot::kDelimiter_
      << client->is_floating() << Snapshot::kDelimiter_
      << client->is_fullscreen() << Snapshot::kDelimiter_
      << client->has_unmap_req_from_wm() << endl;
  }


  // 3. Client Tree serialization.
  for (const auto& workspace : wm->workspaces_) {
    fout << workspace->Serialize() << endl;
  }


  // 4. Curernt Workspace serialization.
  fout << wm->current_ << endl;


  // 5. Docks/notifications serialization.
  if (wm->docks_.empty()) {
    fout << Snapshot::kNone_;
  } else {
    for (size_t i = 0; i < wm->docks_.size(); i++) {
      fout << wm->docks_[i];
      fout << ((i < wm->docks_.size() - 1) ? "," : "");
    }
  }
  fout << endl;

  if (wm->notifications_.empty()) {
    fout << Snapshot::kNone_;
  } else {
    for (size_t i = 0; i < wm->notifications_.size(); i++) {
      fout << wm->notifications_[i];
      fout << ((i < wm->notifications_.size() - 1) ? "," : "");
    }
  }
  fout << endl;
}


const string& Snapshot::filename() const {
  return filename_;
}

} // namespace wmderland
