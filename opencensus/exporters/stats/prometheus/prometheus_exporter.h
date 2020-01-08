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

#ifndef OPENCENSUS_EXPORTERS_STATS_PROMETHEUS_PROMETHEUS_EXPORTER_H_
#define OPENCENSUS_EXPORTERS_STATS_PROMETHEUS_PROMETHEUS_EXPORTER_H_

#include <vector>

#include "absl/strings/string_view.h"
#include "opencensus/stats/stats.h"
#include "prometheus/collectable.h"
#include "prometheus/metric_family.h"

namespace opencensus {
namespace exporters {
namespace stats {

// The PrometheusExporter is a Collectable that exposes all views registered
// with the opencensus StatsExporter to the Prometheus cpp client library. To
// use with the Prometheus client library:
//
//   Exposer exposer("[::]:8080");
//   auto exporter = std::make_shared<PrometheusExporter>();
//   exposer.RegisterCollectable(exporter);
//
// Alternatively, client applications that do not use the default Exposer can
// call Collect() directly and use the serializers in the Prometheus client
// library to expose their own Prometheus endpoint.
//
// PrometheusExporter is thread-safe.
class PrometheusExporter final : public ::prometheus::Collectable {
 public:
  std::vector<prometheus::MetricFamily> Collect() const override;
};

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_STATS_PROMETHEUS_PROMETHEUS_EXPORTER_H_
