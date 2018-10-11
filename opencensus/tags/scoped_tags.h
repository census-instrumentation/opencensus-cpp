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

#ifndef OPENCENSUS_TAGS_SCOPED_TAGS_H_
#define OPENCENSUS_TAGS_SCOPED_TAGS_H_

#include <initializer_list>
#include <utility>

#include "absl/strings/string_view.h"
#include "opencensus/tags/tag_key.h"
#include "opencensus/tags/tag_map.h"

namespace opencensus {
namespace tags {

// ScopedTags is an RAII object, only ever stack-allocate it. While it's in
// scope, the current Context will execute with the specified tags added. If a
// tag key was already in use, its value will be replaced.
class ScopedTags {
 public:
  explicit ScopedTags(
      std::initializer_list<std::pair<TagKey, absl::string_view>> tags);
  ~ScopedTags();

 private:
  ScopedTags() = delete;
  ScopedTags(const ScopedTags&) = delete;
  ScopedTags(ScopedTags&&) = delete;
  ScopedTags& operator=(const ScopedTags&) = delete;
  ScopedTags& operator=(ScopedTags&&) = delete;

  TagMap swapped_tag_map_;
};

// Catch a bug where no name is given and the object is immediately discarded.
#define ScopedTags(x)                                                     \
  do {                                                                    \
    static_assert(                                                        \
        false,                                                            \
        "ScopedTags needs to be an object on the stack and have a name"); \
  } while (0)

}  // namespace tags
}  // namespace opencensus

#endif  // OPENCENSUS_TAGS_SCOPED_TAGS_H_
