// Copyright 2019, OpenCensus Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "opencensus/common/internal/hostname.h"

#include <unistd.h>

#include "absl/strings/str_cat.h"

namespace opencensus {
namespace common {

std::string Hostname() {
  // SUSv2 says 255 is the limit for hostnames.
  char buf[256];
  if (gethostname(buf, sizeof(buf)) == -1) {
    return "unknown_hostname";
  }
  // gethostname() doesn't guarantee NUL termination.
  buf[sizeof(buf) - 1] = 0;
  return buf;
}

std::string OpenCensusTask() {
  return absl::StrCat("cpp-", getpid(), "@", Hostname());
}

}  // namespace common
}  // namespace opencensus
