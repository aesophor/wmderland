#include "cookie.hpp"
#include "util.hpp"
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
    : dpy_(dpy), prop_(prop), filename_ (filename) {
    filename_ = sys_utils::ToAbsPath(filename_);
    std::ifstream file(filename_);
    string line;

    while (std::getline(file, line)) {
        string_utils::Strip(line);

        if (!line.empty()) {
            int res_class_name_length;
            string res_class_name;
            Area window_area;

            // First, check the length of the res_class_name string, and 
            // use res_class_name_tokens[1].substr() to extract it accurately.
            vector<string> res_class_name_tokens = string_utils::Split(line, kDelimiter, 1);
            stringstream(res_class_name_tokens[0]) >> res_class_name_length;
            res_class_name = res_class_name_tokens[1].substr(0, res_class_name_length);

            string pos_size_str = res_class_name_tokens[1].substr(res_class_name_length + 1, string::npos);
            vector<string> pos_size_tokens = string_utils::Split(pos_size_str, kDelimiter);
            stringstream(pos_size_tokens[0]) >> window_area.x;
            stringstream(pos_size_tokens[1]) >> window_area.y;
            stringstream(pos_size_tokens[2]) >> window_area.width;
            stringstream(pos_size_tokens[3]) >> window_area.height;

            window_area_map_[res_class_name] = window_area;
        }
    }

    file.close();
}

Cookie::~Cookie() {}


Area Cookie::Get(Window w) const {
    XClassHint hint = wm_utils::GetXClassHint(dpy_, w);
    string net_wm_name = wm_utils::GetNetWmName(dpy_, w, prop_);
    string cookie_key = string(hint.res_class) + "," + hint.res_name + "," + net_wm_name;

    if (window_area_map_.find(cookie_key) != window_area_map_.end()) {
        return window_area_map_.at(cookie_key);
    }
    return Area(0, 0, 0, 0);
}

void Cookie::Put(Window w, const Area& window_area) {
    XClassHint hint = wm_utils::GetXClassHint(dpy_, w);
    string net_wm_name = wm_utils::GetNetWmName(dpy_, w, prop_);
    string cookie_key = string(hint.res_class) + "," + hint.res_name + "," + net_wm_name;

    window_area_map_[cookie_key] = window_area;
    WriteToFile();
}

void Cookie::WriteToFile() const {
    std::ofstream file(filename_);

    for (auto wps : window_area_map_) {
        file << wps.first.length() << kDelimiter << wps.first << kDelimiter
            << wps.second.x << kDelimiter << wps.second.y << kDelimiter
            << wps.second.width << kDelimiter << wps.second.height << endl;
    }

    file.close();
}
