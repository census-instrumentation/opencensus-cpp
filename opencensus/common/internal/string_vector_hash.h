// Copyright 2017, OpenCensus Authors
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

#ifndef OPENCENSUS_COMMON_INTERNAL_STRING_VECTOR_HASH_H_
#define OPENCENSUS_COMMON_INTERNAL_STRING_VECTOR_HASH_H_

#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace opencensus {
namespace common {

struct StringVectorHash {
  std::size_t operator()(const std::vector<std::string>& container) const {
    std::hash<std::string> hasher;
    size_t hash = 1;
    for (const auto& elem : container) {
      static const size_t kMul = static_cast<size_t>(0xdc3eb94af8ab4c93ULL);
      hash *= kMul;
      hash = ((hash << 19) |
              (hash >> (std::numeric_limits<size_t>::digits - 19))) +
             hasher(elem);
    }
    return hash;
  }
};

}  // namespace common
}  // namespace opencensus

#endif  // OPENCENSUS_COMMON_INTERNAL_STRING_VECTOR_HASH_H_
