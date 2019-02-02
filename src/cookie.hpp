// Cookie holds the user-prefered positions and sizes
// of windows (these data are stored in XSizeHints struct).
//
// When the Window Manager starts, these data will be loaded
// into window_pos_size_map_ from a file.
//
// When the Window Manager shutdowns, these data will be written
// back to the file from window_pos_size_map_.

#ifndef WMDERLAND_COOKIE_HPP_
#define WMDERLAND_COOKIE_HPP_

#include "util.hpp"
extern "C" {
#include <X11/Xutil.h>
}
#include <string>
#include <unordered_map>

class Cookie {
public:
    static const char kDelimiter = ' ';
    Cookie(const std::string filename);
    virtual ~Cookie();

    WindowPosSize Get(const std::string& res_class_name) const;
    void Put(const std::string& res_class_name, WindowPosSize window_pos_size);
    void WriteToFile() const;

private:
    std::string filename_;
    std::unordered_map<std::string, WindowPosSize> window_pos_size_map_;
};

#endif
