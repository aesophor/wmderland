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
    filename_ = string_utils::ToAbsPath(filename_);

    std::ifstream file(filename_);
    string line;

    while (std::getline(file, line)) {
        string_utils::Trim(line);

        if (!line.empty()) {
            string res_class_name;
            WindowPosSize window_pos_size;

            vector<string> tokens = string_utils::Split(line, ' ');
            res_class_name = tokens[0];
            stringstream(tokens[1]) >> window_pos_size.x;
            stringstream(tokens[2]) >> window_pos_size.y;
            stringstream(tokens[3]) >> window_pos_size.width;
            stringstream(tokens[4]) >> window_pos_size.height;

            window_pos_size_map_[res_class_name] = window_pos_size;
        }
    }

    file.close();
}


WindowPosSize Cookie::Get(const string& res_class_name) {
    if (window_pos_size_map_.find(res_class_name) != window_pos_size_map_.end()) {
        return window_pos_size_map_[res_class_name];
    }
    return WindowPosSize();
}

void Cookie::Put(const string& res_class_name, WindowPosSize window_pos_size) {
    window_pos_size_map_[res_class_name] = window_pos_size;
    WriteToFile();
}

void Cookie::WriteToFile() {
    std::ofstream file(filename_);

    for (auto wps : window_pos_size_map_) {
        file << wps.first << " "
            << wps.second.x << " " << wps.second.y << " "
            << wps.second.width << " " << wps.second.height << endl;
    }

    file.close();
}
