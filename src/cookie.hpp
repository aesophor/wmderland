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

class Properties;

class Cookie {
public:
    static const char kDelimiter = ' ';
    Cookie(Display* dpy, Properties* prop, const std::string filename);
    virtual ~Cookie();

    Area Get(Window w) const;
    void Put(Window w, const Area& window_area);
    void WriteToFile() const;

private:
    Display* dpy_;
    Properties* prop_;

    std::string filename_;
    std::unordered_map<std::string, Area> window_area_map_;
};

#endif
