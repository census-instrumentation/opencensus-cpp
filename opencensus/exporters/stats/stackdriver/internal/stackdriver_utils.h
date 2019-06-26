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

#ifndef OPENCENSUS_EXPORTERS_STATS_INTERNAL_STACKDRIVER_UTILS_H_
#define OPENCENSUS_EXPORTERS_STATS_INTERNAL_STACKDRIVER_UTILS_H_

#include <vector>

#include "absl/strings/string_view.h"
#include "google/api/metric.pb.h"
#include "google/monitoring/v3/metric.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

// Populates metric_descriptor. project_name should be in the format
// "projects/project_id".
void SetMetricDescriptor(
    absl::string_view project_name, absl::string_view metric_domain,
    const opencensus::stats::ViewDescriptor& view_descriptor,
    google::api::MetricDescriptor* metric_descriptor);

// Converts each row of 'data' into TimeSeries.
std::vector<google::monitoring::v3::TimeSeries> MakeTimeSeries(
    absl::string_view metric_domain, absl::string_view resource_type,
    const opencensus::stats::ViewDescriptor& view_descriptor,
    const opencensus::stats::ViewData& data, absl::string_view opencensus_task);

// Populates proto based on the given time.
void SetTimestamp(absl::Time time, google::protobuf::Timestamp* proto);

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_STATS_INTERNAL_STACKDRIVER_UTILS_H_
