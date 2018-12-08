#include "data.hpp"
#include <fstream>
#include <glog/logging.h>

using std::pair;
using std::string;
using std::unordered_map;

Data::Data(const string filename) : filename_ (filename) {
    /*
    std::ifstream file(filename_);
    string line;

    while (std::getline(file, line)) {
        
    }
    */
}


WindowPosSize Data::Get(const string& res_class_name) {
    if (attr_map_.find(res_class_name) != attr_map_.end()) {
        return attr_map_[res_class_name];
    }
    return WindowPosSize();
}

void Data::Put(const string& res_class_name, WindowPosSize window_pos_size) {
    attr_map_[res_class_name] = window_pos_size;
}
