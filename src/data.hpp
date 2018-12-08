#ifndef DATA_HPP_
#define DATA_HPP_

#include "util.hpp"
#include <X11/Xutil.h>
#include <string>
#include <unordered_map>

/* The Data class holds the user-prefered positions and sizes
 * of windows (these data are stored in XSizeHints struct).
 *
 * When the Window Manager starts, these data will be loaded
 * into attr_map_ from a file.
 *
 * When the Window Manager shutdowns, these data will be written
 * back to the file from attr_map_.
 */
class Data {
public:
    Data(const std::string filename);

    WindowPosSize Get(const std::string& res_class_name);
    void Put(const std::string& res_class_name, WindowPosSize window_pos_size);

private:
    const std::string filename_;
    std::unordered_map<std::string, WindowPosSize> attr_map_;
};

#endif
