// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_STACKTRACE_H_
#define WMDERLAND_STACKTRACE_H_

namespace wmderland {

namespace segv {

void InstallHandler(void (*Handler)(int));
void Handle(int);

}  // namespace segv

}  // namespace wmderland

#endif  // WMDERLAND_STACKTRACE_H_
