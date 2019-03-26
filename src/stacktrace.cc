#include "stacktrace.h"

namespace {
  int stacktrace_function_count = 10;
  char* stacktrace_log_location;
} // namespace


namespace segv {

void InstallHandler(void (*Handler)(int),
                    int stacktrace_function_count,
                    char* stacktrace_log_location) {
  ::stacktrace_function_count = stacktrace_function_count;
  ::stacktrace_log_location = stacktrace_log_location;

  signal(SIGSEGV, Handler);
}

void Handle(int sig) {
  void* array[10];
  size_t size = backtrace(array, 10);

  int fd = open("/tmp/Wmderland.stacktrace.log", O_CREAT | O_WRONLY, 0600);
  backtrace_symbols_fd(array + 2, size - 2, fd);
  close(fd);

  exit(EXIT_FAILURE);
}

} // namespace segv
