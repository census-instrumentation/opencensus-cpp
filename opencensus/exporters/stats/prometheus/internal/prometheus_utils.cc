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
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/macros.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "opencensus/stats/stats.h"
#include "prometheus/metric_type.h"

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

prometheus::MetricType MetricType(opencensus::stats::Aggregation::Type type) {
  switch (type) {
    case opencensus::stats::Aggregation::Type::kCount:
      return prometheus::MetricType::Counter;
    case opencensus::stats::Aggregation::Type::kSum:
      return prometheus::MetricType::Untyped;
    case opencensus::stats::Aggregation::Type::kLastValue:
      return prometheus::MetricType::Gauge;
    case opencensus::stats::Aggregation::Type::kDistribution:
      return prometheus::MetricType::Histogram;
  }
  ABSL_ASSERT(false && "Bad MetricType.");
  return prometheus::MetricType::Untyped;
}

void SetValue(double value, prometheus::MetricType type,
              prometheus::ClientMetric* metric) {
  if (type == prometheus::MetricType::Untyped) {
    metric->untyped.value = value;
  } else {
    ABSL_ASSERT(type == prometheus::MetricType::Gauge);
    metric->gauge.value = value;
  }
}

void SetValue(int64_t value, prometheus::MetricType type,
              prometheus::ClientMetric* metric) {
  switch (type) {
    case prometheus::MetricType::Counter: {
      metric->counter.value = value;
      break;
    }
    case prometheus::MetricType::Gauge: {
      metric->gauge.value = value;
      break;
    }
    case prometheus::MetricType::Untyped: {
      metric->untyped.value = value;
      break;
    }
    default:
      ABSL_ASSERT(false && "Invalid MetricType for int64 value.");
  }
}

void SetValue(const opencensus::stats::Distribution& value,
              prometheus::MetricType type ABSL_ATTRIBUTE_UNUSED,
              prometheus::ClientMetric* metric) {
  auto& histogram = metric->histogram;
  histogram.sample_count = value.count();
  histogram.sample_sum = value.count() * value.mean();

  int64_t cumulative_count = 0;
  histogram.bucket.reserve(value.bucket_boundaries().num_buckets());
  for (int i = 0; i < value.bucket_boundaries().num_buckets(); ++i) {
    cumulative_count += value.bucket_counts()[i];
    histogram.bucket.emplace_back();
    histogram.bucket[i].cumulative_count = cumulative_count;
    // We use lower boundaries plus an underflow bucket; Prometheus uses upper
    // boundaries, including a +Inf boundary.
    histogram.bucket[i].upper_bound =
        i < value.bucket_boundaries().lower_boundaries().size()
            ? value.bucket_boundaries().lower_boundaries()[i]
            : std::numeric_limits<double>::infinity();
  }
}

template <typename T>
void SetData(const opencensus::stats::ViewDescriptor& descriptor,
             const opencensus::stats::ViewData::DataMap<T>& data, int64_t time,
             prometheus::MetricType type,
             prometheus::MetricFamily* metric_family) {
  metric_family->metric.reserve(data.size());
  for (const auto& row : data) {
    metric_family->metric.emplace_back();
    prometheus::ClientMetric& metric = metric_family->metric.back();
    metric.timestamp_ms = time;
    metric.label.resize(descriptor.num_columns());
    for (int i = 0; i < descriptor.num_columns(); ++i) {
      metric.label[i].name = SanitizeName(descriptor.columns()[i].name());
      metric.label[i].value = row.first[i];
    }
    SetValue(row.second, type, &metric);
  }
}

}  // namespace

void SetMetricFamily(const opencensus::stats::ViewDescriptor& descriptor,
                     const opencensus::stats::ViewData& data,
                     prometheus::MetricFamily* metric_family) {
  const prometheus::MetricType type =
      MetricType(descriptor.aggregation().type());
  // TODO(sturdy): convert common units into base units (e.g. ms->s).
  metric_family->name = SanitizeName(absl::StrCat(
      descriptor.name(), "_", descriptor.measure_descriptor().units()));
  metric_family->help = descriptor.description();
  metric_family->type = type;

  const int64_t time = absl::ToUnixMillis(data.end_time());
  switch (data.type()) {
    case opencensus::stats::ViewData::Type::kDouble: {
      SetData(descriptor, data.double_data(), time, type, metric_family);
      break;
    }
    case opencensus::stats::ViewData::Type::kInt64: {
      SetData(descriptor, data.int_data(), time, type, metric_family);
      break;
    }
    case opencensus::stats::ViewData::Type::kDistribution: {
      SetData(descriptor, data.distribution_data(), time, type, metric_family);
      break;
    }
  }
}

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
