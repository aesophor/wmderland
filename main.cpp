#include "wm.hpp"
#include <glog/logging.h>

int main(int argc, char** args) {
    google::InitGoogleLogging(args[0]);

    std::unique_ptr<WindowManager> wm = WindowManager::GetInstance();

    if (wm == None) {
        return EXIT_FAILURE;
    }

    wm->Run();
    
    return EXIT_SUCCESS;
}
