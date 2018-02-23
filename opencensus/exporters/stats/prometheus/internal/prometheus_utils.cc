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

#include "opencensus/exporters/stats/prometheus/internal/prometheus_utils.h"

#include <cctype>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "metrics.pb.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

namespace {

// Replaces non-alphanumeric characters with underscores to satisfy
// Prometheus's name requirements.
std::string SanitizeName(absl::string_view name) {
  std::string sanitized(name);
  std::replace_if(sanitized.begin(), sanitized.end(),
                  [](char c) { return !::isalnum(c); }, '_');
  return sanitized;
}

io::prometheus::client::MetricType MetricType(
    opencensus::stats::Aggregation::Type type) {
  switch (type) {
    case opencensus::stats::Aggregation::Type::kCount:
      return io::prometheus::client::MetricType::COUNTER;
    case opencensus::stats::Aggregation::Type::kSum:
      return io::prometheus::client::MetricType::UNTYPED;
    case opencensus::stats::Aggregation::Type::kDistribution:
      return io::prometheus::client::MetricType::HISTOGRAM;
  }
}

void SetValue(double value, io::prometheus::client::Metric* metric) {
  metric->mutable_untyped()->set_value(value);
}
void SetValue(int64_t value, io::prometheus::client::Metric* metric) {
  metric->mutable_counter()->set_value(value);
}
void SetValue(const opencensus::stats::Distribution& value,
              io::prometheus::client::Metric* metric) {
  auto* histogram = metric->mutable_histogram();
  histogram->set_sample_count(value.count());
  histogram->set_sample_sum(value.count() * value.mean());

  int64_t cumulative_count = 0;
  for (int i = 0; i < value.bucket_boundaries().num_buckets(); ++i) {
    cumulative_count += value.bucket_counts()[i];
    auto* bucket = histogram->add_bucket();
    bucket->set_cumulative_count(cumulative_count);
    // We use lower boundaries plus an underflow bucket; Prometheus uses upper
    // boundaries, including a +Inf boundary.
    bucket->set_upper_bound(
        i < value.bucket_boundaries().lower_boundaries().size()
            ? value.bucket_boundaries().lower_boundaries()[i]
            : std::numeric_limits<double>::infinity());
  }
}

template <typename T>
void SetData(const opencensus::stats::ViewDescriptor& descriptor,
             const opencensus::stats::ViewData::DataMap<T>& data, int64_t time,
             io::prometheus::client::MetricFamily* metric_family) {
  for (const auto& row : data) {
    auto* metric = metric_family->add_metric();
    metric->set_timestamp_ms(time);
    for (int i = 0; i < descriptor.num_columns(); ++i) {
      auto* label = metric->add_label();
      label->set_name(SanitizeName(descriptor.columns()[i]));
      label->set_value(row.first[i]);
    }
    SetValue(row.second, metric);
  }
}

}  // namespace

void SetMetricFamily(const opencensus::stats::ViewDescriptor& descriptor,
                     const opencensus::stats::ViewData& data,
                     io::prometheus::client::MetricFamily* metric_family) {
  // TODO(sturdy): convert common units into base units (e.g. ms->s).
  metric_family->set_name(SanitizeName(absl::StrCat(
      descriptor.name(), "_", descriptor.measure_descriptor().units())));
  metric_family->set_help(descriptor.description());
  metric_family->set_type(MetricType(descriptor.aggregation().type()));

  const int64_t time = absl::ToUnixMillis(data.end_time());
  switch (data.type()) {
    case opencensus::stats::ViewData::Type::kDouble: {
      SetData(descriptor, data.double_data(), time, metric_family);
      break;
    }
    case opencensus::stats::ViewData::Type::kInt64: {
      SetData(descriptor, data.int_data(), time, metric_family);
      break;
    }
    case opencensus::stats::ViewData::Type::kDistribution: {
      SetData(descriptor, data.distribution_data(), time, metric_family);
      break;
    }
  }
}

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
