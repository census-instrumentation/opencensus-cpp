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

#include "opencensus/trace/exporter/running_span_store.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "gtest/gtest.h"
#include "absl/memory/memory.h"
#include "opencensus/trace/exporter/attribute_value.h"
#include "opencensus/trace/exporter/span_data.h"
#include "opencensus/trace/exporter/status.h"
#include "opencensus/trace/internal/running_span_store_impl.h"
#include "opencensus/trace/internal/span_impl.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"
#include "opencensus/trace/span_context.h"
#include "opencensus/trace/span_id.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/trace_options.h"
#include "opencensus/trace/trace_params.h"

namespace opencensus {
namespace trace {

struct SpanTestPeer {
  static void SetSampled(TraceOptions* opts, bool is_sampled) {
    opts->SetSampled(is_sampled);
  }
};

namespace exporter {
struct RunningSpanStoreImplTestPeer {
  static void ClearForTesting() {
    RunningSpanStoreImpl::Get()->ClearForTesting();
  }
};

namespace {
constexpr uint8_t trace_id[] = {1, 2,  3,  4,  5,  6,  7,  8,
                                9, 10, 11, 12, 13, 14, 15, 16};
constexpr uint8_t span_id1[] = {1, 0, 0, 0, 0, 0, 0, 11};
constexpr uint8_t span_id2[] = {2, 0, 0, 0, 0, 0, 0, 22};
constexpr uint8_t span_id3[] = {3, 0, 0, 0, 0, 0, 0, 33};

TEST(RunningSpanStoreTest, ForceSamplingViaStartSpanOptions) {
  // Parent isn't sampled, child is.
  // Child Span with forced sampling.
  SpanContext parent_ctx2{TraceId(trace_id), SpanId(span_id2)};
  AlwaysSampler sampler;
  RunningSpanStoreImplTestPeer::ClearForTesting();
  auto span2 =
      Span::StartSpanWithRemoteParent("Span2", parent_ctx2, {&sampler});
  EXPECT_TRUE(span2.IsSampled());
  EXPECT_EQ(1, RunningSpanStore::GetRunningSpans({"", 1}).size());
  EXPECT_EQ(0, RunningSpanStore::GetRunningSpans({"", 0}).size())
      << "Hit max_spans_to_return.";
  span2.End();
  EXPECT_EQ(0, RunningSpanStore::GetRunningSpans({"", 1}).size());
}

TEST(RunningSpanStoreTest, ForceSamplingOffViaTraceConfig) {
  // No sampling requested, but trace_params forces it.
  TraceConfig::SetCurrentTraceParams(
      TraceParams{32, 32, 128, 128, ProbabilitySampler(0.0)});
  for (int i = 0; i < 1000; ++i) {
    auto span = Span::StartSpan("SpanName");
    EXPECT_FALSE(span.IsSampled());
    span.End();
  }
}

TEST(RunningSpanStoreTest, GetSummaryAndGetRunningSpans) {
  AlwaysSampler sampler;
  StartSpanOptions opts = {&sampler};
  RunningSpanStoreImplTestPeer::ClearForTesting();
  auto span1 = Span::StartSpan("Group1", nullptr, opts);
  EXPECT_TRUE(span1.IsSampled());

  auto span2 = Span::StartSpan("Group1", nullptr, opts);
  EXPECT_TRUE(span2.IsSampled());

  auto span3 = Span::StartSpan("Group2", nullptr, opts);
  EXPECT_TRUE(span3.IsSampled());

  auto summary = RunningSpanStore::GetSummary();
  EXPECT_EQ(2, summary.per_span_name_summary["Group1"].num_running_spans);
  EXPECT_EQ(1, summary.per_span_name_summary["Group2"].num_running_spans);

  EXPECT_EQ(2, RunningSpanStore::GetRunningSpans({"Group1", 10}).size());
  EXPECT_EQ(1, RunningSpanStore::GetRunningSpans({"Group2", 10}).size());
  EXPECT_EQ(0, RunningSpanStore::GetRunningSpans({"GroupX", 10}).size());

  span1.End();
  summary = RunningSpanStore::GetSummary();
  EXPECT_EQ(1, summary.per_span_name_summary["Group1"].num_running_spans);
  EXPECT_EQ(1, summary.per_span_name_summary["Group2"].num_running_spans);
}

}  // namespace
}  // namespace exporter
}  // namespace trace
}  // namespace opencensus
