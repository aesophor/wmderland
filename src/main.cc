#include "wm.h"
#include "config.h"
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#if GLOG_FOUND
#include <glog/logging.h>
#endif

using std::string;
using std::unique_ptr;

string version() {
    return WIN_MGR_NAME " " VERSION "\n"
        "Copyright (C) 2018-2019 Marco Wang <m.aesophor@gmail.com>\n"
        "This is free software, see the source for copying conditions.  There is No\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\n";
}

int main(int argc, char* args[]) {
    if (argc > 1 && (!strcmp(args[1], "-v") || !strcmp(args[1], "--version"))) {
        std::cout << version();
        return EXIT_SUCCESS;
    }

    try {
        // Initialize google's c++ logging library.
        #if GLOG_FOUND
            google::InitGoogleLogging(args[0]);
        #endif

        // WindowManager is a singleton class. If XOpenDisplay() fails during 
        // WindowManager::GetInstance(), it will return None (in xlib, None is
        // the universal null resource ID or atom.)
        unique_ptr<WindowManager> wm = WindowManager::GetInstance();
        if (!wm) {
            WM_LOG(INFO, "Failed to open display to X server.");
            return EXIT_FAILURE;
        }
        wm->Run();
    } catch (const std::exception& ex) {
        WM_LOG(ERROR, ex.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
