#include "cookie.h"

#include <sstream>
#include <vector>

#include "util.h"

using std::endl;
using std::pair;
using std::vector;
using std::string;
using std::ofstream;
using std::ifstream;
using std::stringstream;
using std::unordered_map;

Cookie::Cookie(Display* dpy, Properties* prop, string filename)
    : dpy_(dpy), prop_(prop), filename_(filename) {
  // Load cookie from file.
  ifstream fin(sys_utils::ToAbsPath(filename_));
  fin >> *this;
  fin.close();
}

Cookie::~Cookie() {}


Area Cookie::Get(Window w) const {
  string key = GetCookieKey(w);

  if (window_area_map_.find(key) != window_area_map_.end()) {
    return window_area_map_.at(key);
  }
  return Area();
}

void Cookie::Put(Window w, const Area& window_area) {
  window_area_map_[GetCookieKey(w)] = window_area;

  // Write cookie to file.
  ofstream fout(filename_);
  fout << *this;
  fout.close();
}

string Cookie::GetCookieKey(Window w) const {
  pair<string, string> hint = wm_utils::GetXClassHint(w);
  string net_wm_name = wm_utils::GetNetWmName(w);
  return hint.first + ',' + hint.second + ',' + net_wm_name;
}


ofstream& operator<< (ofstream& ofs, const Cookie& cookie) {
  for (auto& area : cookie.window_area_map_) {
    // Write x, y, width, height, res_class,res_name,net_wm_name to cookie.
    ofs << area.second.x << Cookie::kDelimiter
      << area.second.y << Cookie::kDelimiter
      << area.second.width << Cookie::kDelimiter
      << area.second.height << Cookie::kDelimiter
      << area.first << endl;
  }

  return ofs;
}

ifstream& operator>> (ifstream& ifs, Cookie& cookie) {
  string line;

  while (std::getline(ifs, line)) {
    string_utils::Strip(line);

    if (!line.empty()) {
      vector<string> tokens = string_utils::Split(line, Cookie::kDelimiter, 4);

      // The first 4 item is x, y, width, height of a window.
      Area window_area;
      stringstream(tokens[0]) >> window_area.x;
      stringstream(tokens[1]) >> window_area.y;
      stringstream(tokens[2]) >> window_area.width;
      stringstream(tokens[3]) >> window_area.height;

      // The rest will be res_class, res_name and _NET_WM_NAME.
      cookie.window_area_map_[tokens[4]] = window_area;
    }
  }

  return ifs;
}
