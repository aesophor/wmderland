#ifndef WMDERLAND_STACKTRACE_H_
#define WMDERLAND_STACKTRACE_H_

extern "C" {
#include <execinfo.h> // backtrace*
#include <signal.h> // signal
#include <stdlib.h> // exit
#include <unistd.h> // close
#include <fcntl.h> // open
}


namespace segv {

void InstallHandler(void (*Handler)(int),
                    int stacktrace_function_count=10,
                    char* stacktrace_log_location="/tmp/Wmderland.STACKTRACE");

void Handle(int sig);

} // namespace segv

#endif // WMDERLAND_STACKTRACE_H_
