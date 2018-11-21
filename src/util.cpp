#include "util.hpp"

namespace wm_utils {

    XWindowAttributes QueryWindowAttributes(Display* dpy, Window w) {
        XWindowAttributes ret;
        XGetWindowAttributes(dpy, w, &ret);
        return ret;
    }

    std::string QueryWmClass(Display* dpy, Window w) {
        XClassHint hint;
        XGetClassHint(dpy, w, &hint);
        return std::string(hint.res_class);
    }

    bool IsBar(const std::string& wm_class) {
        return wm_class == "Polybar";
    }

    bool IsBar(Display* dpy, Window w) {
        return IsBar(QueryWmClass(dpy, w));
    }

}
