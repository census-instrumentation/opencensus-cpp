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

#include "opencensus/stats/tag_set.h"

#include <functional>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "opencensus/common/internal/hash_mix.h"

namespace opencensus {
namespace stats {

TagSet::TagSet(
    std::initializer_list<std::pair<absl::string_view, absl::string_view>>
        tags) {
  tags_.reserve(tags.size());
  for (const auto& tag : tags) {
    tags_.emplace_back(std::string(tag.first), std::string(tag.second));
  }
  Initialize();
}

TagSet::TagSet(std::vector<std::pair<std::string, std::string>> tags)
    : tags_(std::move(tags)) {
  Initialize();
}

void TagSet::Initialize() {
  std::sort(tags_.begin(), tags_.end());

  std::hash<std::string> hasher;
  common::HashMix mixer;
  for (const auto& tag : tags_) {
    mixer.Mix(hasher(tag.first));
    mixer.Mix(hasher(tag.second));
  }
  hash_ = mixer.get();
}

std::size_t TagSet::Hash::operator()(const TagSet& tag_set) const {
  return tag_set.hash_;
}

bool TagSet::operator==(const TagSet& other) const {
  return tags_ == other.tags_;
}

}  // namespace stats
}  // namespace opencensus
