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

#include <iostream>
#include <memory>

#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"
#include "opencensus/stats/stats.h"
#include "prometheus/exposer.h"

int main(int argc, char** argv) {
  // Expose a Prometheus endpoint and register the OpenCensus exporter with it.
  prometheus::Exposer exposer("127.0.0.1:8080");
  auto exporter =
      std::make_shared<opencensus::exporters::stats::PrometheusExporter>();
  exposer.RegisterCollectable(exporter);

  const auto key1 = opencensus::tags::TagKey::Register("key1");
  const auto key2 = opencensus::tags::TagKey::Register("key2");

  // Create a view and register it with the exporter.
  const std::string foo_usage_measure_name = "example.com/Foo/FooUsage";
  const opencensus::stats::MeasureDouble foo_usage =
      opencensus::stats::MeasureDouble::Register(foo_usage_measure_name,
                                                 "Usage of foos.", "foos");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name("example.com/Bar/FooUsage-sum-cumulative-key1-key2")
          .set_measure(foo_usage_measure_name)
          .set_aggregation(opencensus::stats::Aggregation::Distribution(
              opencensus::stats::BucketBoundaries::Explicit({0, 10})))
          .add_column(key1)
          .add_column(key2)
          .set_description(
              "Cumulative distribution of example.com/Foo/FooUsage broken down "
              "by 'key1' and 'key2'.");
  view_descriptor.RegisterForExport();

  std::cout << "Access metrics on http://127.0.0.1:8080/metrics\n";
  while (true) {
    opencensus::stats::Record({{foo_usage, 1.0}}, {{key1, "v1"}});
    opencensus::stats::Record({{foo_usage, 7.0}}, {{key1, "v1"}});
    opencensus::stats::Record({{foo_usage, 12.0}}, {{key1, "v1"}});
    opencensus::stats::Record({{foo_usage, 5.0}}, {{key1, "v1"}, {key2, "v2"}});
    absl::SleepFor(absl::Seconds(10));
  }
}
