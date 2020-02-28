// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
extern "C" {
#include <unistd.h>
}
#include <cstring>
#include <iostream>
#include <memory>

#include "log.h"
#include "snapshot.h"
#include "stacktrace.h"
#include "util.h"
#include "window_manager.h"

namespace {

const char* version() {
  return WIN_MGR_NAME
      " " VERSION
      "\n"
      "Copyright (C) 2018-2020 Marco Wang <m.aesophor@gmail.com>\n"
      "This is free software, see the source for copying conditions. There is no\n"
      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE";
}

}  // namespace

int main(int argc, char* args[]) {
  if (argc > 1 && (!std::strcmp(args[1], "-v") || !std::strcmp(args[1], "--version"))) {
    std::cout << ::version() << std::endl;
    return EXIT_SUCCESS;
  }

  // Install segv handler which writes stacktrace to a log upon segfault.
  // See stacktrace.cc
  wmderland::segv::InstallHandler(&wmderland::segv::Handle);

  // Initialize google's c++ logging library (if installed)
  // Logging-related macros are defined in config.h.in
  WM_INIT_LOGGING(args[0]);

  // WindowManager is a singleton class. If XOpenDisplay() fails during
  // WindowManager::GetInstance(), it will return None (in Xlib, 'None'
  // is the universal null resource ID or atom.)
  std::unique_ptr<wmderland::WindowManager> wm(wmderland::WindowManager::GetInstance());

  if (!wm) {
    const char* err_msg = "Failed to open display to X server.";
    WM_LOG(INFO, err_msg);
    std::cerr << err_msg << std::endl;
    return EXIT_FAILURE;
  }

  try {
    // Try to perform error recovery from the snapshot if necessary and
    // possible.
    if (wm->snapshot().FileExists()) {
      wm->snapshot().Load();
    }
    wm->Run();  // enter main event loop

  } catch (const std::bad_alloc& ex) {
    static_cast<void>(fputs("Out of memory\n", stderr));
    return EXIT_FAILURE;

  } catch (const wmderland::Snapshot::SnapshotLoadError& ex) {
    // If we cannot recover from errors using the snapshot,
    // then return with EXIT_FAILURE.
    WM_LOG(ERROR, ex.what());
    std::cerr << ex.what() << std::endl;

    const char* old_snapshot_name = wm->snapshot().filename().c_str();
    const char* new_snapshot_name = (wm->snapshot().filename() + ".failed_to_load").c_str();

    if (rename(old_snapshot_name, new_snapshot_name) == -1) {
      WM_LOG_WITH_ERRNO("Failed to rename corrupted snapshot", errno);
    } else if (remove(old_snapshot_name)) {  // returns non-zero on failure
      WM_LOG_WITH_ERRNO("Failed to remove corrupted snapshot", errno);
    }
    return EXIT_FAILURE;

  } catch (const std::exception& ex) {
    // Try to exec itself and recover from errors using the snapshot.
    // If snapshot fails to load, it will throw an SnapshotLoadError.
    // See the previous catch block.
    WM_LOG(ERROR, ex.what());
    wmderland::sys_utils::NotifySend("An error occurred. Recovering...", NOTIFY_SEND_CRITICAL);
    wm->snapshot().Save();
    wm.reset();

    if (execl(args[0], args[0], nullptr) == -1) {
      WM_LOG_WITH_ERRNO("execl() failed", errno);
      return EXIT_FAILURE;
    }

  } catch (...) {
    WM_LOG(ERROR, "Unknown exception caught!");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
