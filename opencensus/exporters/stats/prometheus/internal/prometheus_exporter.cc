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

#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"

#include <utility>
#include <vector>

#include "opencensus/exporters/stats/prometheus/internal/prometheus_utils.h"
#include "opencensus/stats/stats.h"
#include "prometheus/metric_family.h"

namespace opencensus {
namespace exporters {
namespace stats {

std::vector<prometheus::MetricFamily> PrometheusExporter::Collect() const {
  const auto data = opencensus::stats::StatsExporter::GetViewData();
  std::vector<prometheus::MetricFamily> output(data.size());
  for (int i = 0; i < data.size(); ++i) {
    SetMetricFamily(data[i].first, data[i].second, &output[i]);
  }
  return output;
}

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
