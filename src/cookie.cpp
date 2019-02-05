#include "cookie.hpp"
#include <fstream>
#include <sstream>
#include <vector>

using std::endl;
using std::pair;
using std::vector;
using std::string;
using std::stringstream;
using std::unordered_map;

Cookie::Cookie(string filename) : filename_ (filename) {
    filename_ = sys_utils::ToAbsPath(filename_);

    std::ifstream file(filename_);
    string line;

    while (std::getline(file, line)) {
        string_utils::Strip(line);

        if (!line.empty()) {
            int res_class_name_length;
            string res_class_name;
            Area window_pos_size;

            // First, check the length of the res_class_name string, and 
            // use res_class_name_tokens[1].substr() to extract it accurately.
            vector<string> res_class_name_tokens = string_utils::Split(line, kDelimiter, 1);
            stringstream(res_class_name_tokens[0]) >> res_class_name_length;
            res_class_name = res_class_name_tokens[1].substr(0, res_class_name_length);

            string pos_size_str = res_class_name_tokens[1].substr(res_class_name_length + 1, string::npos);
            vector<string> pos_size_tokens = string_utils::Split(pos_size_str, kDelimiter);
            stringstream(pos_size_tokens[0]) >> window_pos_size.x;
            stringstream(pos_size_tokens[1]) >> window_pos_size.y;
            stringstream(pos_size_tokens[2]) >> window_pos_size.width;
            stringstream(pos_size_tokens[3]) >> window_pos_size.height;

            window_area_map_[res_class_name] = window_pos_size;
        }
    }

    file.close();
}

Cookie::~Cookie() {}


Area Cookie::Get(const XClassHint& class_hint, const std::string& wm_name) const {
    string key = string(class_hint.res_class) + ',' + string(class_hint.res_name) + ',' + wm_name;

    if (window_area_map_.find(key) != window_area_map_.end()) {
        return window_area_map_.at(key);
    }
    return Area(0, 0, 0, 0);
}

void Cookie::Put(const string& res_class_name, const Area& window_pos_size) {
    window_area_map_[res_class_name] = window_pos_size;
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
