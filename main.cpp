#include "wm.hpp"
#include <glog/logging.h>

int main(int argc, char** args) {
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
