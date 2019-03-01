#include "stacktrace.h"

extern "C" void segv_init(void (*handler)(int)) {
    signal(SIGSEGV, handler);
}
