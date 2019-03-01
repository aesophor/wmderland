#ifndef WMDERLAND_STACKTRACE_H_
#define WMDERLAND_STACKTRACE_H_

extern "C" {
#include <execinfo.h> // backtrace*
#include <signal.h> // signal
#include <stdlib.h> // exit
#include <unistd.h> // STDERR_FILENO
}

extern "C" void segv_init(void (*handler)(int));
    
#endif
