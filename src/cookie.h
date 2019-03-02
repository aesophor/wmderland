// Cookie holds the user-prefered positions and sizes of windows.

#ifndef WMDERLAND_COOKIE_H_
#define WMDERLAND_COOKIE_H_

extern "C" {
#include <X11/Xutil.h>
}
#include <string>
#include <unordered_map>

#include "util.h"

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
    bool Has(Window w) const;
    std::string GetCookieKey(Window w) const;

    Display* dpy_;
    Properties* prop_;
    std::string filename_;
    std::unordered_map<std::string, Area> window_area_map_;
};

#endif
