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
  const std::vector<std::pair<std::string, std::string>> tags({{"k", "v"}});
  EXPECT_EQ(TagSet(tags), TagSet({{"k", "v"}}));
}

TEST(TagSetTest, TagsSorted) {
  const std::vector<std::pair<std::string, std::string>> expected = {
      {"B", "v"}, {"a", "v"}, {"b", "v"}};
  const std::vector<std::pair<std::string, std::string>> tags(
      {{"b", "v"}, {"a", "v"}, {"B", "v"}});
  EXPECT_THAT(TagSet(tags).tags(), ::testing::ElementsAreArray(expected));
  EXPECT_THAT(TagSet({{"B", "v"}, {"b", "v"}, {"a", "v"}}).tags(),
              ::testing::ElementsAreArray(expected));
}

TEST(TagSetTest, EqualityDisregardsOrder) {
  EXPECT_EQ(TagSet({{"k1", "v1"}, {"k2", "v2"}}),
            TagSet({{"k2", "v2"}, {"k1", "v1"}}));
}

TEST(TagSetTest, EqualityRespectsMissingKeys) {
  EXPECT_NE(TagSet({{"k1", "v1"}, {"k2", "v2"}}), TagSet({{"k1", "v1"}}));
}

TEST(TagSetTest, EqualityRespectsKeyValuePairings) {
  EXPECT_NE(TagSet({{"k1", "v1"}, {"k2", "v2"}}),
            TagSet({{"k1", "v2"}, {"k2", "v1"}}));
}

TEST(TagSetTest, HashDisregardsOrder) {
  TagSet ts1({{"k1", "v1"}, {"k2", "v2"}});
  TagSet ts2({{"k2", "v2"}, {"k1", "v1"}});
  EXPECT_EQ(TagSet::Hash()(ts1), TagSet::Hash()(ts2));
}

TEST(TagSetTest, HashRespectsKeyValuePairings) {
  TagSet ts1({{"k1", "v1"}, {"k2", "v2"}});
  TagSet ts2({{"k1", "v2"}, {"k2", "v1"}});
  EXPECT_NE(TagSet::Hash()(ts1), TagSet::Hash()(ts2));
}

TEST(TagSetTest, UnorderedMap) {
  // Test that the operators and hash are compatible with std::unordered_map.
  std::unordered_map<TagSet, int, TagSet::Hash> map;
  TagSet ts = {{"key", "value"}};
  map.emplace(ts, 1);
  EXPECT_NE(map.end(), map.find(ts));
  EXPECT_EQ(1, map.erase(ts));
}

}  // namespace
}  // namespace stats
}  // namespace opencensus
