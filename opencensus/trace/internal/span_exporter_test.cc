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

#include "opencensus/trace/exporter/span_exporter.h"

#include "absl/memory/memory.h"
#include "absl/synchronization/mutex.h"
#include "gtest/gtest.h"
#include "opencensus/trace/exporter/span_data.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"

namespace opencensus {
namespace trace {
namespace {

class MyExporter : public exporter::SpanExporter::Handler {
 public:
  void Export(const std::vector<exporter::SpanData>& spans) override {
    absl::MutexLock l(&mu_);
    num_exported_ += spans.size();
  }

  int num_exported() const {
    absl::MutexLock l(&mu_);
    return num_exported_;
  }

 private:
  mutable absl::Mutex mu_;
  int num_exported_ GUARDED_BY(mu_) = 0;
};

class SpanExporterTest : public ::testing::Test {
 public:
  SpanExporterTest() : exporter_(new MyExporter) {
    exporter::SpanExporter::Register(
        absl::WrapUnique<exporter::SpanExporter::Handler>(exporter_));
  }

 protected:
  MyExporter* exporter_;
};

TEST_F(SpanExporterTest, BasicExportTest) {
  ::opencensus::trace::AlwaysSampler sampler;
  ::opencensus::trace::StartSpanOptions opts = {&sampler};

  auto span1 = ::opencensus::trace::Span::StartSpan("Span1", nullptr, opts);
  absl::SleepFor(absl::Milliseconds(100));
  auto span2 = ::opencensus::trace::Span::StartSpan("Span2", &span1, opts);
  absl::SleepFor(absl::Milliseconds(200));
  auto span3 = ::opencensus::trace::Span::StartSpan("Span3", &span2);
  absl::SleepFor(absl::Milliseconds(300));
  span3.End();
  span2.End();
  span1.End();

  for (int i = 0; i < 10; ++i) {
    if (exporter_->num_exported() >= 3) break;
    sleep(1);
  }
  EXPECT_EQ(3, exporter_->num_exported());
}

}  // namespace
}  // namespace trace
}  // namespace opencensus
