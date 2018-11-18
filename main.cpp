#include "wm.hpp"

int main(void) {
    std::unique_ptr<WindowManager> wm = WindowManager::GetInstance();

    if (wm == None) {
        return EXIT_FAILURE;
    }

    wm->Run();
    
    return EXIT_SUCCESS;
}
