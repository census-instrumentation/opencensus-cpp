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

#include "opencensus/trace/trace_options.h"

#include "gtest/gtest.h"

namespace opencensus {
namespace trace {

struct SpanTestPeer {
  static void SetSampled(TraceOptions* opts, bool is_sampled) {
    opts->SetSampled(is_sampled);
  }
};

namespace {

TEST(TraceOptionsTest, SetSampled) {
  TraceOptions opts;

  EXPECT_EQ("00", opts.ToHex());
  EXPECT_FALSE(opts.IsSampled());
  SpanTestPeer::SetSampled(&opts, true);
  EXPECT_EQ("01", opts.ToHex());
  EXPECT_TRUE(opts.IsSampled());
  SpanTestPeer::SetSampled(&opts, false);
  EXPECT_FALSE(opts.IsSampled());
}

TEST(TraceOptionsTest, Comparison) {
  TraceOptions a;
  TraceOptions b;
  EXPECT_EQ(a, b);
  SpanTestPeer::SetSampled(&a, true);
  EXPECT_FALSE(a == b);
}

}  // namespace
}  // namespace trace
}  // namespace opencensus
