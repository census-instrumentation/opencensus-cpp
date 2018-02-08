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

#include "opencensus/stats/stats_exporter.h"

#include <cstdint>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "opencensus/stats/measure.h"
#include "opencensus/stats/measure_descriptor.h"
#include "opencensus/stats/measure_registry.h"
#include "opencensus/stats/view_descriptor.h"

namespace opencensus {
namespace stats {

// A mock exporter that checks whether it is called with the right set of view
// descriptors.
class MockExporter : public StatsExporter::Handler {
 public:
  static void Register(absl::Span<const ViewDescriptor> expected_descriptors) {
    opencensus::stats::StatsExporter::RegisterHandler(
        absl::make_unique<MockExporter>(expected_descriptors));
  }

  MockExporter(absl::Span<const ViewDescriptor> expected_descriptors)
      : expected_descriptors_(expected_descriptors.begin(),
                              expected_descriptors.end()) {}

  ~MockExporter() {
    EXPECT_THAT(actual_descriptors_,
                ::testing::UnorderedElementsAreArray(expected_descriptors_));
  }

  void ExportViewData(const ViewDescriptor& descriptor,
                      const ViewData& data) override {
    actual_descriptors_.push_back(descriptor);
  }

 private:
  const std::vector<ViewDescriptor> expected_descriptors_;
  std::vector<ViewDescriptor> actual_descriptors_;
};

constexpr char kMeasureId[] = "test_measure_id";

MeasureDouble TestMeasure() {
  static MeasureDouble measure =
      MeasureRegistry::RegisterDouble(kMeasureId, "ops", "");
  return measure;
}

class StatsExporterTest : public ::testing::Test {
 protected:
  void SetUp() {
    // Access the measure to ensure it has been registered.
    TestMeasure();
    descriptor1_.set_name("id1");
    descriptor1_.set_measure(kMeasureId);
    descriptor1_.set_aggregation(Aggregation::Count());
    descriptor1_.set_aggregation_window(AggregationWindow::Cumulative());
    descriptor1_edited_.set_name("id1");
    descriptor1_edited_.set_measure(kMeasureId);
    descriptor1_edited_.set_aggregation(Aggregation::Sum());
    descriptor1_edited_.set_aggregation_window(AggregationWindow::Cumulative());
    descriptor2_.set_name("id2");
    descriptor2_.set_measure(kMeasureId);
    descriptor2_.set_aggregation(
        Aggregation::Distribution(BucketBoundaries::Explicit({0})));
    descriptor2_.set_aggregation_window(
        AggregationWindow::Interval(absl::Hours(1)));
  }

  void TearDown() {
    StatsExporter::RemoveView(descriptor1_.name());
    StatsExporter::RemoveView(descriptor2_.name());
    StatsExporter::ClearHandlersForTesting();
  }

  static void Export() { StatsExporter::ExportForTesting(); }

  ViewDescriptor descriptor1_;
  ViewDescriptor descriptor1_edited_;
  ViewDescriptor descriptor2_;
};

TEST_F(StatsExporterTest, AddView) {
  MockExporter::Register({descriptor1_, descriptor2_});
  StatsExporter::AddView(descriptor1_);
  StatsExporter::AddView(descriptor2_);
  Export();
}

TEST_F(StatsExporterTest, UpdateView) {
  MockExporter::Register({descriptor1_edited_, descriptor2_});
  StatsExporter::AddView(descriptor1_);
  StatsExporter::AddView(descriptor2_);
  StatsExporter::AddView(descriptor1_edited_);
  Export();
}

TEST_F(StatsExporterTest, RemoveView) {
  MockExporter::Register({descriptor2_});
  StatsExporter::AddView(descriptor1_);
  StatsExporter::AddView(descriptor2_);
  StatsExporter::RemoveView(descriptor1_.name());
  Export();
}

TEST_F(StatsExporterTest, MultipleExporters) {
  MockExporter::Register({descriptor1_});
  MockExporter::Register({descriptor1_});
  StatsExporter::AddView(descriptor1_);
  Export();
}

TEST_F(StatsExporterTest, TimedExport) {
  MockExporter::Register({descriptor1_});
  StatsExporter::AddView(descriptor1_);
  absl::SleepFor(absl::Seconds(11));
}

}  // namespace stats
}  // namespace opencensus
