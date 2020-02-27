// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_LOG_H_
#define WMDERLAND_LOG_H_

#include "config.h"

// If glog is not installed on the compiling machine,
// then these macros will do nothing.
#if GLOG_FOUND
#include <glog/logging.h>
#define WM_INIT_LOGGING(executable_name) google::InitGoogleLogging(executable_name)
#define WM_LOG(severity, msg)                \
  do {                                       \
    LOG(severity) << msg;                    \
    google::FlushLogFiles(google::severity); \
  } while (0)
#else
#define WM_INIT_LOGGING(executable_name)
#define WM_LOG(severity, msg)
#endif


#define WM_LOG_WITH_ERRNO(reason, errno)               \
  do {                                                 \
    const char* err_msg = reason;                      \
    WM_LOG(ERROR, err_msg << ": " << strerror(errno)); \
    perror(err_msg);                                   \
  } while (0)

#endif  // WMDERLAND_LOG_H_
