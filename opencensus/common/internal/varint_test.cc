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

#include "opencensus/common/internal/varint.h"

#include "absl/strings/string_view.h"
#include "gtest/gtest.h"

namespace opencensus {
namespace common {
namespace {

TEST(Varint, EncodeDecode) {
  auto test = [](int i) {
    std::string s;
    AppendVarint(i, &s);
    absl::string_view sv(s);
    int j = -1;
    EXPECT_TRUE(ParseVarint(&sv, &j));
    EXPECT_EQ(i, j);
  };
  test(0);
  test(1);
  test(10);
  test(100);
  test(127);
  test(128);
  test(129);
}

}  // namespace
}  // namespace common
}  // namespace opencensus
