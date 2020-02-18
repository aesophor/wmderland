// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "stacktrace.h"

extern "C" {
#include <stdlib.h>    // exit
#ifdef __GLIBC__
#include <execinfo.h>  // backtrace*
#endif
#include <fcntl.h>     // open
#include <signal.h>    // signal
#include <unistd.h>    // close
}

#define STACKTRACE_LOG "/tmp/Wmderland.STACKTRACE"
#define STACKTRACE_FUNC_COUNT 10

namespace wmderland {

namespace segv {

void InstallHandler(void (*Handler)(int)) {
#ifdef __GLIBC__
  signal(SIGSEGV, Handler);
#endif
}

void Handle(int) {
#ifdef __GLIBC__
  void* array[STACKTRACE_FUNC_COUNT];
  size_t size = backtrace(array, STACKTRACE_FUNC_COUNT);

  int fd = open(STACKTRACE_LOG, O_CREAT | O_WRONLY, 0600);
  backtrace_symbols_fd(array + 2, size - 2, fd);
  close(fd);

  exit(EXIT_FAILURE);
#endif
}

}  // namespace segv

}  // namespace wmderland
