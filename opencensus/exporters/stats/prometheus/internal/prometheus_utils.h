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

#ifndef OPENCENSUS_EXPORTERS_STATS_PROMETHEUS_INTERNAL_PROMETHEUS_UTILS_H_
#define OPENCENSUS_EXPORTERS_STATS_PROMETHEUS_INTERNAL_PROMETHEUS_UTILS_H_

#include <utility>
#include <vector>

#include "opencensus/stats/stats.h"
#include "prometheus/metric_family.h"

namespace opencensus {
namespace exporters {
namespace stats {

// Populates metric_family based on view_descriptor and view_data.
void SetMetricFamily(const opencensus::stats::ViewDescriptor& descriptor,
                     const opencensus::stats::ViewData& data,
                     prometheus::MetricFamily* metric_family);

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_STATS_PROMETHEUS_INTERNAL_PROMETHEUS_UTILS_H_
