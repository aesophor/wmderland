// Cookie holds the user-prefered positions and sizes of windows.

#ifndef WMDERLAND_COOKIE_H_
#define WMDERLAND_COOKIE_H_

#include <string>
#include <fstream>
#include <unordered_map>

extern "C" {
#include <X11/Xutil.h>
}

#include "util.h"

class Properties;

class Cookie {
 public:
  Cookie(Display* dpy, Properties* prop, const std::string filename);
  virtual ~Cookie() = default;

  Area Get(Window w) const;
  void Put(Window w, const Area& window_area);

  friend std::ofstream& operator<< (std::ofstream& os, const Cookie& cookie);
  friend std::ifstream& operator>> (std::ifstream& is, Cookie& cookie);

 private:
  static const char kDelimiter = ' ';
  std::string GetCookieKey(Window w) const;

  Display* dpy_;
  Properties* prop_;
  std::string filename_;
  std::unordered_map<std::string, Area> window_area_map_;
};

#endif
