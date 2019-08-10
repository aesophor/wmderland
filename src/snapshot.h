// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_SNAPSHOT_H_
#define WMDERLAND_SNAPSHOT_H_

#include <string>

namespace wmderland {

class Snapshot {
 public:
  Snapshot(const std::string& filename);
  virtual ~Snapshot() = default;

  void Load() const;
  void Save() const;

 private:
  const std::string filename_;
};

} // namespace wmderland

#endif // WMDERLAND_SNAPSHOT_H_
