#include "cookie.h"

#include <fstream>
#include <sstream>
#include <vector>

#include "util.h"

using std::endl;
using std::pair;
using std::vector;
using std::string;
using std::stringstream;
using std::unordered_map;

Cookie::Cookie(Display* dpy, Properties* prop, string filename)
    : dpy_(dpy), prop_(prop), filename_(filename) {
  filename_ = sys_utils::ToAbsPath(filename_);
  std::ifstream file(filename_);
  string line;

  while (std::getline(file, line)) {
    string_utils::Strip(line);

    if (!line.empty()) {
      vector<string> tokens = string_utils::Split(line, kDelimiter, 4);

      // The first 4 item is x, y, width, height of a window.
      Area window_area;
      stringstream(tokens[0]) >> window_area.x;
      stringstream(tokens[1]) >> window_area.y;
      stringstream(tokens[2]) >> window_area.width;
      stringstream(tokens[3]) >> window_area.height;

      // The rest will be res_class, res_name and _NET_WM_NAME.
      window_area_map_[tokens[4]] = window_area;
    }
  }
  file.close();
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
  WriteToFile();
}

void Cookie::WriteToFile() const {
  std::ofstream file(filename_);

  for (auto& area : window_area_map_) {
    // Write x, y, width, height, res_class,res_name,net_wm_name to cookie.
    file << area.second.x << kDelimiter << area.second.y << kDelimiter
      << area.second.width << kDelimiter << area.second.height << kDelimiter
      << area.first << endl;
  }
  file.close();
}


string Cookie::GetCookieKey(Window w) const {
  pair<string, string> hint = wm_utils::GetXClassHint(w);
  string net_wm_name = wm_utils::GetNetWmName(w);
  return hint.first + ',' + hint.second + ',' + net_wm_name;
}
