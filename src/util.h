// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_UTIL_H_
#define WMDERLAND_UTIL_H_

extern "C" {
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <string>
#include <vector>

#include "properties.h"

namespace wmderland {

namespace wm_utils {

void Init(Display* dpy, Properties* prop, Window root_window);
XWindowAttributes GetXWindowAttributes(Window window);
XSizeHints GetWmNormalHints(Window window);
std::pair<std::string, std::string> GetXClassHint(Window window);
std::string GetNetWmName(Window window);
std::string GetWmName(Window window);
void SetWindowWmState(Window window, unsigned long state);
void SetNetActiveWindow(Window window);
void ClearNetActiveWindow();
Atom* GetWindowProperty(Window window, Atom property, unsigned long* atom_len);
bool WindowPropertyHasAtom(Window window, Atom property, Atom target_atom);

bool HasNetWmStateFullscreen(Window window);
bool IsWindowOfType(Window window, Atom type_atom);
bool IsDock(Window window);
bool IsDialog(Window window);
bool IsSplash(Window window);
bool IsUtility(Window window);
bool IsNotification(Window window);

}  // namespace wm_utils

namespace string_utils {

std::vector<std::string> Split(const std::string& s, const char delimiter);
std::vector<std::string> Split(const std::string& s, const char delimiter, int count);
bool StartsWith(const std::string& s, const std::string& keyword);
bool Contains(const std::string& s, const std::string& keyword);
void Replace(std::string& s, const std::string& keyword, const std::string& newword);
void Strip(std::string& s);

}  // namespace string_utils

#define NOTIFY_SEND_LOW "low"
#define NOTIFY_SEND_NORMAL "normal"
#define NOTIFY_SEND_CRITICAL "critical"

namespace sys_utils {

std::string ToAbsPath(const std::string& path);
void ExecuteCmd(std::string cmd);
void NotifySend(const std::string& msg, const std::string& level = NOTIFY_SEND_NORMAL);

}  // namespace sys_utils

}  // namespace wmderland

#endif  // WMDERLAND_UTIL_H_
