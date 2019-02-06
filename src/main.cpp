#include "wm.hpp"
#include "config.hpp"
#include <glog/logging.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;

void print_version() {
    cout << WIN_MGR_NAME << " " << VERSION << endl
        << "Copyright (C) 2018-2019 Marco Wang <m.aesophor@gmail.com>" << endl
        << "This is free software, see the source for copying conditions.  There is No" << endl
        << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE" << endl;
}

int main(int argc, char* args[]) {
    if (argc > 1 && (!strcmp(args[1], "-v") || !strcmp(args[1], "--version"))) {
        print_version();
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
