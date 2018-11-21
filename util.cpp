#include "util.hpp"

namespace wm_utils {

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
