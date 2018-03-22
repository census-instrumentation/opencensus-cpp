// Copyright 2018, OpenCensus Authors
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

#ifndef OPENCENSUS_STATS_TAG_SET_H_
#define OPENCENSUS_STATS_TAG_SET_H_

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"

namespace opencensus {
namespace stats {

// TagSet represents a set of key-value tags, and provides efficient equality
// and hash operations. A TagSet is expensive to construct, and should be shared
// between uses where possible.
// TagSet is immutable.
class TagSet final {
 public:
  // Both constructors are not explicit so that Record({}, {{"k", "v"}}) works.
  // This constructor is needed because even though we copy to a vector
  // internally because c++ cannot deduce the conversion needed.
  TagSet(std::initializer_list<std::pair<absl::string_view, absl::string_view>>
             tags);
  // This constructor is needed so that callers can dynamically construct
  // tagsets. It takes the argument by value to allow it to be moved.
  TagSet(std::vector<std::pair<std::string, std::string>> tags);

  // Accesses the tags sorted by key.
  const std::vector<std::pair<std::string, std::string>>& tags() const {
    return tags_;
  }

  struct Hash {
    std::size_t operator()(const TagSet& tag_set) const;
  };

  bool operator==(const TagSet& other) const;
  bool operator!=(const TagSet& other) const { return !(*this == other); }

 private:
  void Initialize();

  std::size_t hash_;
  // TODO: add an option to store string_views to avoid copies.
  std::vector<std::pair<std::string, std::string>> tags_;
};

}  // namespace stats
}  // namespace opencensus

#endif  // OPENCENSUS_STATS_TAG_SET_H_
