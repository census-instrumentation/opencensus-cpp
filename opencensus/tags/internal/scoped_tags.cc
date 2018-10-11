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

#include "opencensus/tags/scoped_tags.h"

#include <initializer_list>
#include <utility>

#include "absl/strings/string_view.h"
#include "opencensus/tags/tag_key.h"

// Guard macro from scoped_tags.h.
#undef ScopedTags

namespace opencensus {
namespace tags {

ScopedTags::ScopedTags(
    std::initializer_list<std::pair<TagKey, absl::string_view>> tags)
    : swapped_tag_map_({}) {
  // TODO: Implement this.
}

ScopedTags::~ScopedTags() {
  // TODO: Implement this.
}

}  // namespace tags
}  // namespace opencensus
