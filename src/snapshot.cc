// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "snapshot.h"

#include <fstream>

using std::string;
using std::ifstream;
using std::ofstream;

namespace wmderland {

Snapshot::Snapshot(const string& filename) : filename_(filename) {}


void Snapshot::Load() const {

}

void Snapshot::Save() const {
  ofstream fout(filename_);
}

} // namespace wmderland
