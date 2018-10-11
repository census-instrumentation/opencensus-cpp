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

#include <iostream>

#include "gtest/gtest.h"
//#include "opencensus/context/context.h"
#include "opencensus/tags/tag_key.h"

// Not in namespace ::opencensus::context in order to better reflect what user
// code should look like.

namespace {

void ExpectNoTags() {
  // Implement this.
}

TEST(ScopedTagsTest, NestedScopes) {
  static const auto k1 = opencensus::tags::TagKey::Register("key1");
  static const auto k2 = opencensus::tags::TagKey::Register("key2");
  static const auto k3 = opencensus::tags::TagKey::Register("key3");

  ExpectNoTags();
  {
    opencensus::tags::ScopedTags tags1({{k1, "v1"}});
    // Expect something.
    {
      opencensus::tags::ScopedTags tags2({{k2, "v2"}, {k3, "v3"}});
      // Expect something.
    }
  }
  ExpectNoTags();
}

TEST(ScopedTagsTest, ReplaceValue) {
  static const auto k1 = opencensus::tags::TagKey::Register("key1");
  static const auto k2 = opencensus::tags::TagKey::Register("key2");
  static const auto k3 = opencensus::tags::TagKey::Register("key3");

  // Order shouldn't matter.
  opencensus::tags::ScopedTags tags1({{k2, "v2"}, {k1, "v1"}});
  {
    opencensus::tags::ScopedTags tags2({{k3, "v3"}, {k2, "xx"}});
    // Expect something.
  }
}

// Disable non-compilation test.
#if 0
TEST(ScopedTagsTest, NCForgotName) {
  static const auto k1 = opencensus::tags::TagKey::Register("key1");
  opencensus::tags::ScopedTags({{k1, "v1"}});
}
#endif

}  // namespace
