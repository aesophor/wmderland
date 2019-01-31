#include "wm.hpp"
#include <glog/logging.h>

int main(int argc, char* args[]) {
    // Initialize Google's C++ logging library.
    ::google::InitGoogleLogging(args[0]);

    WindowManager* wm = WindowManager::GetInstance();

    if (!wm) {
        LOG(INFO) << "Failed to open display to X server.";
        return EXIT_FAILURE;
    }
    
    LOG(INFO) << "Wmderland launched.";
    wm->Run();
    delete wm;
    return EXIT_SUCCESS;
}
