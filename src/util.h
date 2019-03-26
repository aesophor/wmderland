#ifndef WMDERLAND_UTIL_H_
#define WMDERLAND_UTIL_H_

#include "properties.h"
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
}
#include <string>
#include <vector>

struct Area {
  Area();
  Area(int x, int y, int width, int height);

  bool operator==(const Area& other);
  bool operator!=(const Area& other);

  int x, y, width, height;
};


namespace tiling {

enum Direction {
  UNSPECIFIED,
  HORIZONTAL,
  VERTICAL
};

} // namespace tiling


namespace wm_utils {

void Init(Display* d, Properties* p, Window root_win);
std::pair<int, int> GetDisplayResolution();
XWindowAttributes GetXWindowAttributes(Window w);
XSizeHints GetWmNormalHints(Window w);
std::pair<std::string, std::string> GetXClassHint(Window w);
std::string GetNetWmName(Window w);
std::string GetWmName(Window w);
void SetWindowWmState(Window w, unsigned long state);
void SetNetActiveWindow(Window w);
void ClearNetActiveWindow();
Atom* GetWindowProperty(Window w, Atom property, unsigned long* atom_len);
bool WindowPropertyHasAtom(Window w, Atom property, Atom target_atom);

bool HasNetWmStateFullscreen(Window w);
bool IsWindowOfType(Window, Atom type_atom);
bool IsDock(Window w);
bool IsDialog(Window w);
bool IsSplash(Window w);
bool IsUtility(Window w);
bool IsNotification(Window w);

std::string KeysymToStr(unsigned int keycode);
unsigned int StrToKeycode(const std::string& key_name);
std::string KeymaskToStr(int modifier);
int StrToKeymask(const std::string& modifier_str, bool shift);

} // namespace wm_utils


namespace string_utils {

std::vector<std::string> Split(const std::string& s, const char delimiter);
std::vector<std::string> Split(const std::string& s, const char delimiter, int count);
bool StartsWith(const std::string& s, const std::string& keyword);
bool Contains(const std::string& s, const std::string& keyword);
void Replace(std::string& s, const std::string& keyword, const std::string& newword);
void Strip(std::string& s);

} // namespace string_utils


namespace sys_utils {

std::string ToAbsPath(const std::string& path);

} // namespace sys_utils

#endif
