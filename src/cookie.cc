#include "cookie.h"
#include "util.h"
#include <fstream>
#include <sstream>
#include <vector>

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
    return (Has(w)) ? window_area_map_.at(GetCookieKey(w)) : Area();
}

void Cookie::Put(Window w, const Area& window_area) {
    window_area_map_[GetCookieKey(w)] = window_area;
    WriteToFile();
}

void Cookie::WriteToFile() const {
    std::ofstream file(filename_);

    for (auto& w : window_area_map_) {
        // Write x, y, width, height, res_class,res_name,net_wm_name to cookie.
        file << w.second.x << kDelimiter << w.second.y << kDelimiter
            << w.second.width << kDelimiter << w.second.height << kDelimiter
            << w.first << endl;
    }

    file.close();
}


bool Cookie::Has(Window w) const {
    return window_area_map_.find(GetCookieKey(w)) != window_area_map_.end();
}

string Cookie::GetCookieKey(Window w) const {
    pair<string, string> hint = wm_utils::GetXClassHint(w);
    string net_wm_name = wm_utils::GetNetWmName(w);
    return hint.first + ',' + hint.second + ',' + net_wm_name;
}
