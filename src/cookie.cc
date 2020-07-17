// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "cookie.h"

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
}

#include <sstream>
#include <vector>

#include "client.h"
#include "util.h"
#include "log.h"

using std::endl;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::vector;

namespace wmderland {

const char Cookie::kDelimiter_ = ' ';

Cookie::Cookie(Display* dpy, Properties* prop, string filename)
    : dpy_(dpy), prop_(prop), filename_(sys_utils::ToAbsPath(filename)) {

  // mkdir ~/.cache/wmderland
  string cookie_dirname = filename_.substr(0, filename_.find_last_of('/'));
  mode_t cookie_dir_mode = 0755;
  mkdir(cookie_dirname.c_str(), cookie_dir_mode);
  WM_LOG(INFO, "Cookie directory: " << cookie_dirname);

  // Load cookie from file.
  ifstream fin(filename_);
  fin >> *this;
}

Client::Area Cookie::Get(Window window) const {
  auto it = client_area_map_.find(GetCookieKey(window));
  if (it != client_area_map_.end()) {
    return it->second;
  }
  return Client::Area();
}

void Cookie::Put(Window window, const Client::Area& area) {
  client_area_map_[GetCookieKey(window)] = area;

  // Write cookie to file.
  ofstream fout(filename_);
  fout << *this;
}

string Cookie::GetCookieKey(Window window) const {
  pair<string, string> hint = wm_utils::GetXClassHint(window);
  string net_wm_name = wm_utils::GetNetWmName(window);
  return hint.first + ',' + hint.second + ',' + net_wm_name;
}

ofstream& operator<<(ofstream& ofs, const Cookie& cookie) {
  for (auto& area : cookie.client_area_map_) {
    // Write x, y, width, height, res_class,res_name,net_wm_name to cookie.
    ofs << area.second.x << Cookie::kDelimiter_ << area.second.y << Cookie::kDelimiter_
        << area.second.w << Cookie::kDelimiter_ << area.second.h << Cookie::kDelimiter_
        << area.first << endl;
  }
  return ofs;
}

ifstream& operator>>(ifstream& ifs, Cookie& cookie) {
  string line;
  while (std::getline(ifs, line)) {
    string_utils::Strip(line);

    if (!line.empty()) {
      vector<string> tokens = string_utils::Split(line, Cookie::kDelimiter_, 4);

      // The first 4 item is x, y, width, height of a window.
      Client::Area area;
      stringstream(tokens[0]) >> area.x;
      stringstream(tokens[1]) >> area.y;
      stringstream(tokens[2]) >> area.w;
      stringstream(tokens[3]) >> area.h;

      // The rest will be res_class, res_name and _NET_WM_NAME.
      cookie.client_area_map_[tokens[4]] = area;
    }
  }
  return ifs;
}

}  // namespace wmderland
