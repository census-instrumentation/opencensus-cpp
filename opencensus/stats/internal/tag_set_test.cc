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

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace opencensus {
namespace stats {
namespace {

TEST(TagSetTest, ConstructorsEquivalent) {
  TagKey key = TagKey::Register("k");
  const std::vector<std::pair<TagKey, std::string>> tags({{key, "v"}});
  EXPECT_EQ(TagSet(tags), TagSet({{key, "v"}}));
}

TEST(TagSetTest, TagsSorted) {
  TagKey k1 = TagKey::Register("b");
  TagKey k2 = TagKey::Register("c");
  TagKey k3 = TagKey::Register("a");
  const std::vector<std::pair<TagKey, std::string>> expected = {
      {k1, "v"}, {k2, "v"}, {k3, "v"}};
  const std::vector<std::pair<TagKey, std::string>> tags(
      {{k2, "v"}, {k3, "v"}, {k1, "v"}});
  EXPECT_THAT(TagSet(tags).tags(), ::testing::ElementsAreArray(expected));
  EXPECT_THAT(TagSet({{k2, "v"}, {k1, "v"}, {k3, "v"}}).tags(),
              ::testing::ElementsAreArray(expected));
}

TEST(TagSetTest, EqualityDisregardsOrder) {
  TagKey k1 = TagKey::Register("k1");
  TagKey k2 = TagKey::Register("k2");
  EXPECT_EQ(TagSet({{k1, "v1"}, {k2, "v2"}}), TagSet({{k2, "v2"}, {k1, "v1"}}));
}

TEST(TagSetTest, EqualityRespectsMissingKeys) {
  TagKey k1 = TagKey::Register("k1");
  TagKey k2 = TagKey::Register("k2");
  EXPECT_NE(TagSet({{k1, "v1"}, {k2, "v2"}}), TagSet({{k1, "v1"}}));
}

TEST(TagSetTest, EqualityRespectsKeyValuePairings) {
  TagKey k1 = TagKey::Register("k1");
  TagKey k2 = TagKey::Register("k2");
  EXPECT_NE(TagSet({{k1, "v1"}, {k2, "v2"}}), TagSet({{k1, "v2"}, {k2, "v1"}}));
}

TEST(TagSetTest, HashDisregardsOrder) {
  TagKey k1 = TagKey::Register("k1");
  TagKey k2 = TagKey::Register("k2");
  TagSet ts1({{k1, "v1"}, {k2, "v2"}});
  TagSet ts2({{k2, "v2"}, {k1, "v1"}});
  EXPECT_EQ(TagSet::Hash()(ts1), TagSet::Hash()(ts2));
}

TEST(TagSetTest, HashRespectsKeyValuePairings) {
  TagKey k1 = TagKey::Register("k1");
  TagKey k2 = TagKey::Register("k2");
  TagSet ts1({{k1, "v1"}, {k2, "v2"}});
  TagSet ts2({{k1, "v2"}, {k2, "v1"}});
  EXPECT_NE(TagSet::Hash()(ts1), TagSet::Hash()(ts2));
}

TEST(TagSetTest, UnorderedMap) {
  // Test that the operators and hash are compatible with std::unordered_map.
  TagKey key = TagKey::Register("key");
  std::unordered_map<TagSet, int, TagSet::Hash> map;
  TagSet ts = {{key, "value"}};
  map.emplace(ts, 1);
  EXPECT_NE(map.end(), map.find(ts));
  EXPECT_EQ(1, map.erase(ts));
}

}  // namespace
}  // namespace stats
}  // namespace opencensus
