// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_SNAPSHOT_H_
#define WMDERLAND_SNAPSHOT_H_

extern "C" {
#include <X11/Xlib.h>
}
#include <array>
#include <string>
#include <exception>

#include "config.h"
#include "workspace.h"

namespace wmderland {

class Snapshot {
 public:
  class SnapshotLoadError : public std::exception {
   public:
    virtual ~SnapshotLoadError() = default;
    virtual const char* what() const throw() {
      return "Failed to load snapshot (possibly corrupted). Giving up...";
    }
  };

  Snapshot(const std::string& filename);
  virtual ~Snapshot() = default;

  bool FileExists() const;
  void Load() const;
  void Save() const;

  static const std::string kNone_;
  static const std::string kBacktrack_;
  static const char kLeafPrefix_;
  static const char kInternalPrefix_;
 
 private:
  static const char kDelimiter_;

  const std::string filename_;
};

} // namespace wmderland

#endif // WMDERLAND_SNAPSHOT_H_
