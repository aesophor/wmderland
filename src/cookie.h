// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_COOKIE_H_
#define WMDERLAND_COOKIE_H_

extern "C" {
#include <X11/Xutil.h>
}
#include <fstream>
#include <string>
#include <unordered_map>

#include "client.h"

namespace wmderland {

struct Properties;

// Cookie holds the user-prefered positions and sizes of windows.
class Cookie {
 public:
  Cookie(/*Display* dpy, Properties* prop, */ const std::string & filename);
  virtual ~Cookie() = default;

  Client::Area Get(Window window) const;
  void Put(Window window, const Client::Area& area);

  friend std::ofstream& operator<<(std::ofstream& os, const Cookie& cookie);
  friend std::ifstream& operator>>(std::ifstream& is, Cookie& cookie);

 private:
  static const char kDelimiter_;
  std::string GetCookieKey(Window window) const;

//  Display* dpy_;
//  Properties* prop_;
  std::string filename_;
  std::unordered_map<std::string, Client::Area> client_area_map_;
};

}  // namespace wmderland

#endif  // WMDERLAND_COOKIE_H_
