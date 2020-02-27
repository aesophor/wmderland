// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#include "util.h"

#include <sstream>

#include "log.h"

using std::pair;
using std::size_t;
using std::string;
using std::vector;

namespace {
Display* dpy;
wmderland::Properties* prop;
Window root_window;
}  // namespace

namespace wmderland {

namespace wm_utils {

void Init(Display* dpy, Properties* prop, Window root_window) {
  ::dpy = dpy;
  ::prop = prop;
  ::root_window = root_window;
}

// Get the XWindowAttributes of a window.
XWindowAttributes GetXWindowAttributes(Window window) {
  XWindowAttributes ret;
  XGetWindowAttributes(dpy, window, &ret);
  return ret;
}

// Get the XSizeHints of a window.
XSizeHints GetWmNormalHints(Window window) {
  XSizeHints hints;
  long msize;
  XGetWMNormalHints(dpy, window, &hints, &msize);
  return hints;
}

// Get the XClassHint (which contains res_class and res_name) of a window.
pair<string, string> GetXClassHint(Window window) {
  XClassHint hint;

  if (XGetClassHint(dpy, window, &hint)) {
    static const string undefined = "undefined";
    string res_class = (hint.res_class) ? hint.res_class : undefined;
    string res_name = (hint.res_name) ? hint.res_name : undefined;

    if (hint.res_class) {
      XFree(hint.res_class);
    }
    if (hint.res_name) {
      XFree(hint.res_name);
    }
    return std::make_pair(res_class, res_name);
  }

  return std::make_pair("", "");
}

// Get the utf8string in _NET_WM_NAME property.
string GetNetWmName(Window window) {
  XTextProperty name;
  if (!XGetTextProperty(dpy, window, &name, prop->net[atom::NET_WM_NAME]) || !name.nitems) {
    return "";
  }
  string ret(reinterpret_cast<char*>(name.value));
  XFree(name.value);
  return ret;
}

// Get the WM_NAME (i.e., the window title) of a window.
string GetWmName(Window window) {
  Atom prop = XInternAtom(dpy, "WM_NAME", False), type;
  int form;
  unsigned long remain, len;
  unsigned char* list;

  if (XGetWindowProperty(dpy, window, prop, 0, 1024, False, AnyPropertyType, &type, &form,
                         &len, &remain, &list) == Success) {
    return string(reinterpret_cast<char*>(list));
  }
  return "";
}

// Set WM_STATE according to the following page to fix WINE application close
// hang issue:
// http://www.x.org/releases/X11R7.7/doc/xorg-docs/icccm/icccm.html#WM_STATE_Property
void SetWindowWmState(Window window, unsigned long state) {
  unsigned long wm_state[] = {state, None};
  XChangeProperty(dpy, window, prop->wm[atom::WM_STATE], prop->wm[atom::WM_STATE], 32,
                  PropModeReplace, reinterpret_cast<unsigned char*>(wm_state), 2);
}

// Set root window's _NET_ACTIVE_WINDOW property
void SetNetActiveWindow(Window window) {
  XChangeProperty(dpy, root_window, prop->net[atom::NET_ACTIVE_WINDOW], XA_WINDOW, 32,
                  PropModeReplace, reinterpret_cast<unsigned char*>(&window), 1);
}

// Clear root window's _NET_ACTIVE_WINDOW property
void ClearNetActiveWindow() {
  XDeleteProperty(dpy, root_window, prop->net[atom::NET_ACTIVE_WINDOW]);
}

// Get the atoms contained in the property of window w. The number of atoms
// retrieved will be stored in *atom_len. XFree() should be called manually on
// the returned Atom ptr.
Atom* GetWindowProperty(Window window, Atom property, unsigned long* atom_len) {
  Atom da;
  unsigned char* prop_ret = nullptr;
  int di;
  unsigned long remain;

  if (XGetWindowProperty(dpy, window, property, 0, sizeof(Atom), False, XA_ATOM, &da, &di,
                         atom_len, &remain, &prop_ret) == Success &&
      prop_ret) {
    return reinterpret_cast<Atom*>(prop_ret);
  }
  return nullptr;
}

// Check if the property of window w contains the target atom.
bool WindowPropertyHasAtom(Window window, Atom property, Atom target_atom) {
  unsigned long atom_len = 0;
  Atom* atoms = GetWindowProperty(window, property, &atom_len);

  for (int i = 0; atoms && i < (int)atom_len; i++) {
    if (atoms[i] && atoms[i] == target_atom) {
      XFree(atoms);
      return true;
    }
  }
  XFree(atoms);
  return false;
}

bool HasNetWmStateFullscreen(Window window) {
  return WindowPropertyHasAtom(window, prop->net[atom::NET_WM_STATE],
                               prop->net[atom::NET_WM_STATE_FULLSCREEN]);
}

bool IsWindowOfType(Window window, Atom type_atom) {
  return WindowPropertyHasAtom(window, prop->net[atom::NET_WM_WINDOW_TYPE], type_atom);
}

bool IsDock(Window window) {
  return IsWindowOfType(window, prop->net[atom::NET_WM_WINDOW_TYPE_DOCK]);
}

bool IsDialog(Window window) {
  return IsWindowOfType(window, prop->net[atom::NET_WM_WINDOW_TYPE_DIALOG]);
}

bool IsSplash(Window window) {
  return IsWindowOfType(window, prop->net[atom::NET_WM_WINDOW_TYPE_SPLASH]);
}

bool IsUtility(Window window) {
  return IsWindowOfType(window, prop->net[atom::NET_WM_WINDOW_TYPE_UTILITY]);
}

bool IsNotification(Window window) {
  return IsWindowOfType(window, prop->net[atom::NET_WM_WINDOW_TYPE_NOTIFICATION]);
}

}  // namespace wm_utils

namespace string_utils {

vector<string> Split(const string& s, const char delimiter) {
  std::stringstream ss(s);
  string t;
  vector<string> tokens;

  while (std::getline(ss, t, delimiter)) {
    if (t.length() > 0) {
      tokens.push_back(t);
    }
  }
  return tokens;
}

vector<string> Split(const string& s, const char delimiter, int count) {
  vector<string> tokens;
  string::size_type head = 0;
  string::size_type tail = s.find(delimiter, head);

  for (short i = 0; i < count; i++) {
    string t = s.substr(head, tail - head);
    if (t.length() > 0) {
      tokens.push_back(t);
    }
    head = tail + 1;
    tail = s.find(delimiter, tail + 1);
  }

  if (head != 0) {
    string t = s.substr(head, string::npos);
    if (t.length() > 0) {
      tokens.push_back(t);
    }
  }
  return tokens;
}

bool StartsWith(const string& s, const string& keyword) {
  return s.find(keyword) == 0;
}

bool Contains(const string& s, const string& keyword) {
  return s.find(keyword) != string::npos;
}

void Replace(string& s, const string& keyword, const string& newword) {
  string::size_type pos = s.find(keyword);

  while (pos != std::string::npos) {
    s.replace(pos, keyword.size(), newword);
    pos = s.find(keyword, pos + newword.size());
  }
}

void Strip(string& s) {
  static const char* whitespace_chars = " \n\r\t";
  s.erase(0, s.find_first_not_of(whitespace_chars));
  s.erase(s.find_last_not_of(whitespace_chars) + 1);
}

}  // namespace string_utils

namespace sys_utils {

string ToAbsPath(const string& path) {
  if (path.empty()) {
    return "";
  }

  string abs_path = path;
  if (path.front() == '~') {
    abs_path = string(getenv("HOME")) + path.substr(1, string::npos);
  }
  return abs_path;
}

void ExecuteCmd(string cmd) {
  if (cmd.empty()) {
    return;
  }

  string_utils::Strip(cmd);
  if (cmd.back() != '&') {
    cmd.push_back('&');
  }

  if (system(cmd.c_str())) {
    WM_LOG(ERROR, "Failed to execute: " + cmd);
  }
}

void NotifySend(const string& msg, const string& level) {
  ExecuteCmd("notify-send -u " + level + " 'wmderland' '" + msg + "'");
}

}  // namespace sys_utils

}  // namespace wmderland
