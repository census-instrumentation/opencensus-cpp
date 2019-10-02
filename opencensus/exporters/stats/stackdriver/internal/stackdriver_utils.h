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

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "google/api/metric.pb.h"
#include "google/api/monitored_resource.pb.h"
#include "google/monitoring/v3/metric.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

// Returns a MetricType string. metric_name_prefix must have a trailing slash,
// e.g. "custom.googleapis.com/opencensus/".
std::string MakeType(absl::string_view metric_name_prefix,
                     absl::string_view view_name);

// Returns true if the metric type is a heuristically known
// custom (i.e not built-in) Stackdriver metric.
bool IsKnownCustomMetric(absl::string_view metric_type);

// Returns a pointer to the MonitoredResource proto for this view, or nullptr if
// the default resource should be used.
const google::api::MonitoredResource* MonitoredResourceForView(
    const opencensus::stats::ViewDescriptor& view_descriptor,
    const google::api::MonitoredResource& monitored_resource,
    const std::unordered_map<std::string, google::api::MonitoredResource>&
        per_metric_monitored_resource);

// Populates metric_descriptor. project_name must be in the format
// "projects/project_id". metric_name_prefix must have a trailing slash, e.g.
// "custom.googleapis.com/opencensus/".
void SetMetricDescriptor(
    absl::string_view project_name, absl::string_view metric_name_prefix,
    const opencensus::stats::ViewDescriptor& view_descriptor,
    bool add_task_label, google::api::MetricDescriptor* metric_descriptor);

// Converts each row of 'data' into a TimeSeries proto.
std::vector<google::monitoring::v3::TimeSeries> MakeTimeSeries(
    absl::string_view metric_name_prefix,
    const google::api::MonitoredResource* monitored_resource_for_view,
    const opencensus::stats::ViewDescriptor& view_descriptor,
    const opencensus::stats::ViewData& data, bool add_task_label,
    absl::string_view opencensus_task);

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_STATS_INTERNAL_STACKDRIVER_UTILS_H_
