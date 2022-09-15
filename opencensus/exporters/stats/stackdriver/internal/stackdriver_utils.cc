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

#include "opencensus/exporters/stats/stackdriver/internal/stackdriver_utils.h"

#include <string>

#include "absl/base/internal/sysinfo.h"
#include "absl/base/macros.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "google/api/distribution.pb.h"
#include "google/api/label.pb.h"
#include "google/api/metric.pb.h"
#include "google/api/monitored_resource.pb.h"
#include "google/monitoring/v3/common.pb.h"
#include "google/monitoring/v3/metric.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "opencensus/common/internal/timestamp.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

namespace {

constexpr char kLabelDescription[] = "OpenCensus TagKey";
constexpr char kOpenCensusTaskKey[] = "opencensus_task";
constexpr char kOpenCensusTaskDescription[] = "OpenCensus task identifier";
constexpr char kDefaultResourceType[] = "global";

// Creates a name in the format described in
// https://cloud.google.com/monitoring/api/ref_v3/rest/v3/projects.metricDescriptors/create
std::string MakeName(absl::string_view project_name,
                     absl::string_view metric_name_prefix,
                     absl::string_view view_name) {
  return absl::StrCat(project_name, "/metricDescriptors/",
                      MakeType(metric_name_prefix, view_name));
}

void SetLabelDescriptor(absl::string_view tag_key,
                        google::api::LabelDescriptor* label_descriptor) {
  label_descriptor->set_key(std::string(tag_key));
  label_descriptor->set_value_type(google::api::LabelDescriptor::STRING);
  label_descriptor->set_description(kLabelDescription);
}

void SetOpenCensusTaskLabelDescriptor(
    google::api::LabelDescriptor* label_descriptor) {
  label_descriptor->set_key(kOpenCensusTaskKey);
  label_descriptor->set_value_type(google::api::LabelDescriptor::STRING);
  label_descriptor->set_description(kOpenCensusTaskDescription);
}

google::api::MetricDescriptor::ValueType GetValueType(
    const opencensus::stats::ViewDescriptor& descriptor) {
  switch (descriptor.aggregation().type()) {
    case opencensus::stats::Aggregation::Type::kCount:
      return google::api::MetricDescriptor::INT64;
    case opencensus::stats::Aggregation::Type::kSum:
    case opencensus::stats::Aggregation::Type::kLastValue:
      switch (descriptor.measure_descriptor().type()) {
        case opencensus::stats::MeasureDescriptor::Type::kDouble:
          return google::api::MetricDescriptor::DOUBLE;
        case opencensus::stats::MeasureDescriptor::Type::kInt64:
          return google::api::MetricDescriptor::INT64;
        default:
          ABSL_ASSERT(false && "Unknown measure_descriptor type.");
          return google::api::MetricDescriptor::DOUBLE;
      }
    case opencensus::stats::Aggregation::Type::kDistribution:
      return google::api::MetricDescriptor::DISTRIBUTION;
  }
  ABSL_ASSERT(false && "Bad descriptor type.");
  return google::api::MetricDescriptor::DOUBLE;
}

// Overloaded function for converting ViewData value types to Points. The
// ValueType is needed because Sum aggregation with an int64 measure returns
// doubles but we want to export int64s for future compatibility.
void SetTypedValue(double value, google::api::MetricDescriptor::ValueType type,
                   google::monitoring::v3::TypedValue* proto) {
  if (type == google::api::MetricDescriptor::DOUBLE) {
    proto->set_double_value(value);
  } else {
    ABSL_ASSERT(type == google::api::MetricDescriptor::INT64);
    proto->set_int64_value(static_cast<int64_t>(value));
  }
}
void SetTypedValue(int64_t value, google::api::MetricDescriptor::ValueType type,
                   google::monitoring::v3::TypedValue* proto) {
  ABSL_ASSERT(type == google::api::MetricDescriptor::INT64);
  proto->set_int64_value(value);
}
void SetTypedValue(const opencensus::stats::Distribution& value,
                   google::api::MetricDescriptor::ValueType type,
                   google::monitoring::v3::TypedValue* proto) {
  ABSL_ASSERT(type == google::api::MetricDescriptor::DISTRIBUTION);
  auto* distribution_proto = proto->mutable_distribution_value();
  distribution_proto->set_count(value.count());
  distribution_proto->set_mean(value.mean());
  distribution_proto->set_sum_of_squared_deviation(
      value.sum_of_squared_deviation());
  // TODO: Set range when Stackdriver supports it.
  if (value.bucket_boundaries().num_buckets() > 1) {
    auto* buckets = distribution_proto->mutable_bucket_options()
                        ->mutable_explicit_buckets();
    for (const auto boundary : value.bucket_boundaries().lower_boundaries()) {
      buckets->add_bounds(boundary);
    }
    for (const auto bucket_count : value.bucket_counts()) {
      distribution_proto->add_bucket_counts(bucket_count);
    }
  }
}

template <typename DataValueT>
std::vector<google::monitoring::v3::TimeSeries> DataToTimeSeries(
    const opencensus::stats::ViewDescriptor& view_descriptor,
    const opencensus::stats::ViewData::DataMap<DataValueT>& data,
    const opencensus::stats::ViewData::DataMap<absl::Time>& start_times,
    const google::monitoring::v3::TimeSeries& base_time_series) {
  const google::api::MetricDescriptor::ValueType type =
      GetValueType(view_descriptor);
  std::vector<google::monitoring::v3::TimeSeries> vector;
  for (const auto& row : data) {
    vector.push_back(base_time_series);
    auto& time_series = vector.back();
    for (int i = 0; i < view_descriptor.columns().size(); ++i) {
      (*time_series.mutable_metric()
            ->mutable_labels())[view_descriptor.columns()[i].name()] =
          row.first[i];
    }
    // The point is already created in the base_time_series to set the times.
    SetTypedValue(row.second, type,
                  time_series.mutable_points(0)->mutable_value());

    // Stackdriver doesn't like start_time and end_time being different for
    // GAUGE metrics. Don't set the start time for GAUGE.
    if (view_descriptor.aggregation().type() !=
        opencensus::stats::Aggregation::Type::kLastValue) {
      // Use the start time stored for the specific row's tags.
      auto* interval = time_series.mutable_points(0)->mutable_interval();
      absl::Time start_time = start_times.at(row.first);
      opencensus::common::SetTimestamp(start_time,
                                       interval->mutable_start_time());
    }
  }
  return vector;
}

}  // namespace

std::string MakeType(absl::string_view metric_name_prefix,
                     absl::string_view view_name) {
  return absl::StrCat(metric_name_prefix, view_name);
}

bool IsKnownCustomMetric(absl::string_view metric_type) {
  return absl::StartsWith(metric_type, "custom.googleapis.com/") ||
         absl::StartsWith(metric_type, "external.googleapis.com/") ||
         absl::StartsWith(metric_type, "workload.googleapis.com/");
}

const google::api::MonitoredResource* MonitoredResourceForView(
    const opencensus::stats::ViewDescriptor& view_descriptor,
    const google::api::MonitoredResource& monitored_resource,
    const std::unordered_map<std::string, google::api::MonitoredResource>&
        per_metric_monitored_resource) {
  auto iter = per_metric_monitored_resource.find(view_descriptor.name());
  if (iter != per_metric_monitored_resource.end()) {
    return &iter->second;
  } else if (monitored_resource.type().empty()) {
    return nullptr;
  } else {
    return &monitored_resource;
  }
}

void SetMetricDescriptor(
    absl::string_view project_name, absl::string_view metric_name_prefix,
    const opencensus::stats::ViewDescriptor& view_descriptor,
    bool add_task_label, google::api::MetricDescriptor* metric_descriptor) {
  metric_descriptor->set_name(
      MakeName(project_name, metric_name_prefix, view_descriptor.name()));
  const std::string metric_type =
      MakeType(metric_name_prefix, view_descriptor.name());
  metric_descriptor->set_type(metric_type);
  if (add_task_label) {
    SetOpenCensusTaskLabelDescriptor(metric_descriptor->add_labels());
  }
  for (const auto& tag_key : view_descriptor.columns()) {
    SetLabelDescriptor(tag_key.name(), metric_descriptor->add_labels());
  }
  metric_descriptor->set_metric_kind(
      view_descriptor.aggregation().type() ==
              opencensus::stats::Aggregation::Type::kLastValue
          ? google::api::MetricDescriptor::GAUGE
          : google::api::MetricDescriptor::CUMULATIVE);
  metric_descriptor->set_value_type(GetValueType(view_descriptor));
  metric_descriptor->set_unit(
      view_descriptor.aggregation() == opencensus::stats::Aggregation::Count()
          ? "1"
          : view_descriptor.measure_descriptor().units());
  metric_descriptor->set_description(view_descriptor.description());
}

std::vector<google::monitoring::v3::TimeSeries> MakeTimeSeries(
    absl::string_view metric_name_prefix,
    const google::api::MonitoredResource* monitored_resource_for_view,
    const opencensus::stats::ViewDescriptor& view_descriptor,
    const opencensus::stats::ViewData& data, bool add_task_label,
    absl::string_view opencensus_task) {
  // Set values that are common across all the rows.
  auto base_time_series = google::monitoring::v3::TimeSeries();
  const std::string metric_type =
      MakeType(metric_name_prefix, view_descriptor.name());
  base_time_series.mutable_metric()->set_type(metric_type);
  if (monitored_resource_for_view == nullptr) {
    base_time_series.mutable_resource()->set_type(kDefaultResourceType);
  } else {
    *base_time_series.mutable_resource() = *monitored_resource_for_view;
  }
  auto* interval = base_time_series.add_points()->mutable_interval();
  opencensus::common::SetTimestamp(data.end_time(),
                                   interval->mutable_end_time());
  if (add_task_label) {
    (*base_time_series.mutable_metric()->mutable_labels())[kOpenCensusTaskKey] =
        std::string(opencensus_task);
  }
  switch (data.type()) {
    case opencensus::stats::ViewData::Type::kDouble:
      return DataToTimeSeries(view_descriptor, data.double_data(),
                              data.start_times(), base_time_series);
    case opencensus::stats::ViewData::Type::kInt64:
      return DataToTimeSeries(view_descriptor, data.int_data(),
                              data.start_times(), base_time_series);
    case opencensus::stats::ViewData::Type::kDistribution:
      return DataToTimeSeries(view_descriptor, data.distribution_data(),
                              data.start_times(), base_time_series);
  }
  ABSL_ASSERT(false && "Bad ViewData.type().");
  return {};
}

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
