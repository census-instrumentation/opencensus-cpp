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

#include <iostream>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gtest/gtest.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace stats {

class StatsExporterTest {
 public:
  static constexpr auto& ExportForTesting = StatsExporter::ExportForTesting;
};

}  // namespace stats
}  // namespace opencensus

namespace opencensus_examples {
namespace {

// The resource owner publicly defines their measure IDs.
constexpr char kFooUsageMeasureName[] = "example.com/Foo/FooUsage";

opencensus::stats::MeasureDouble FooUsage() {
  static const opencensus::stats::MeasureDouble foo_usage =
      opencensus::stats::MeasureRegistry::RegisterDouble(
          kFooUsageMeasureName, "foos", "Usage of foos.");
  return foo_usage;
}

// An example exporter that exports to stdout.
class ExampleExporter : public opencensus::stats::StatsExporter::Handler {
 public:
  static void Register() {
    opencensus::stats::StatsExporter::RegisterHandler(
        absl::make_unique<ExampleExporter>());
  }

  void ExportViewData(const opencensus::stats::ViewDescriptor& descriptor,
                      const opencensus::stats::ViewData& data) override {
    if (data.type() != opencensus::stats::ViewData::Type::kDouble) {
      // This example only supports double data (i.e. Sum() aggregation).
      return;
    }
    std::string output;
    absl::StrAppend(&output, "\nData for view \"", descriptor.name(),
                    "\" from ", absl::FormatTime(data.start_time()), " to ",
                    absl::FormatTime(data.end_time()), ":\n");
    for (const auto& row : data.double_data()) {
      for (int i = 0; i < descriptor.columns().size(); ++i) {
        absl::StrAppend(&output, descriptor.columns()[i], ":", row.first[i],
                        ", ");
      }
      absl::StrAppend(&output, row.second, "\n");
    }
    std::cout << output;
  }
};

class ExporterExample : public ::testing::Test {
 protected:
  void SetupFoo() { FooUsage(); }

  void UseFoo(absl::string_view id, double quantity) {
    opencensus::stats::Record({{FooUsage(), quantity}}, {{"foo_id", id}});
  }
};

TEST_F(ExporterExample, Distribution) {
  // Measure initialization must precede view creation.
  SetupFoo();

  // The stats consumer creates a View on Foo Usage.
  const auto cumulative_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name("example.com/Bar/FooUsage-sum-cumulative-foo_id")
          .set_measure(kFooUsageMeasureName)
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .set_aggregation_window(
              opencensus::stats::AggregationWindow::Cumulative())
          .add_column("foo_id")
          .set_description(
              "Cumulative sum of example.com/Foo/FooUsage broken down "
              "by 'foo_id'.");
  const auto interval_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name("example.com/Bar/FooUsage-sum-interval-foo_id")
          .set_measure(kFooUsageMeasureName)
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .set_aggregation_window(
              opencensus::stats::AggregationWindow::Interval(absl::Hours(1)))
          .add_column("foo_id")
          .set_description(
              "Rolling sum of example.com/Foo/FooUsage over the previous hour "
              "broken down by 'foo_id'.");

  // The order of view registration and exporter creation does not matter, as
  // long as both precede data recording.
  opencensus::stats::StatsExporter::AddView(cumulative_descriptor);
  ExampleExporter::Register();
  opencensus::stats::StatsExporter::AddView(interval_descriptor);

  // Someone calls the Foo API, recording usage under example.com/Bar/FooUsage.
  UseFoo("foo1", 1);
  UseFoo("foo1", 4);
  UseFoo("foo2", 2);
  // Stats are exported every 10 seconds.
  absl::SleepFor(absl::Seconds(11));

  UseFoo("foo1", 1);
  UseFoo("foo2", 5);
  absl::SleepFor(absl::Seconds(11));
}

}  // namespace
}  // namespace opencensus_examples
