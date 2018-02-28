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

#include "opencensus/trace/exporter/local_span_store.h"

#include "gtest/gtest.h"
#include "opencensus/trace/internal/local_span_store_impl.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"

namespace opencensus {
namespace trace {

class SpanTestPeer {
 public:
  static void End(absl::Duration latency, Span* span) {
    // Call Span::End to get into stores.
    span->End();
    // Set latency.
    span->span_impl_for_test()->EndWithLatencyForTesting(latency);
  }
};

namespace exporter {
class LocalSpanStoreImplTestPeer {
 public:
  static void ClearForTesting() {
    LocalSpanStoreImpl::Get()->ClearForTesting();
  }
};

namespace {

TEST(LocalSpanStoreTest, GetSummary) {
  exporter::LocalSpanStoreImplTestPeer::ClearForTesting();
  auto span = Span::StartSpan("SpanName", /*parent=*/nullptr,
                              {nullptr, /*record_events=*/true});
  span.AddAnnotation("Annotation");
  SpanTestPeer::End(absl::Microseconds(15), &span);

  auto summary = LocalSpanStore::GetSummary();
  EXPECT_EQ(1, summary.per_span_name_summary.size());
  EXPECT_EQ(1, summary.per_span_name_summary["SpanName"]
                   .number_of_latency_sampled_spans.size());
  EXPECT_EQ(
      1,
      summary.per_span_name_summary["SpanName"].number_of_latency_sampled_spans
          [LocalSpanStore::LatencyBucketBoundary::k10us_to_100us]);
}

}  // namespace
}  // namespace exporter
}  // namespace trace
}  // namespace opencensus
