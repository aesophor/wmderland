#include "wm.hpp"
#include "config.hpp"
#include <glog/logging.h>
#include <iostream>
#include <string>

using std::cout;
using std::string;

string version() {
    return string(WIN_MGR_NAME) + " " + VERSION + "\n"
        + "Copyright (C) 2018-2019 Marco Wang <m.aesophor@gmail.com>\n"
        + "This is free software, see the source for copying conditions.  There is No\n"
        + "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE\n";
}

int main(int argc, char* args[]) {
    if (argc > 1 && (!strcmp(args[1], "-v") || !strcmp(args[1], "--version"))) {
        cout << version();
        return EXIT_SUCCESS;
    }

    // Initialize google's c++ logging library.
    google::InitGoogleLogging(args[0]);

    // WindowManager is a singleton class. If XOpenDisplay() fails during 
    // WindowManager::GetInstance(), it will return None (in xlib, None is
    // the universal null resource ID or atom.)
    WindowManager* wm = WindowManager::GetInstance();
    
    if (!wm) {
        LOG(INFO) << "Failed to open display to X server.";
        return EXIT_FAILURE;
    }

    wm->Run();
    delete wm;
    return EXIT_SUCCESS;
}
