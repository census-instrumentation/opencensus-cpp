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

#include "opencensus/exporters/stats/stackdriver/internal/time_series_matcher.h"

#include <cstdint>
#include <map>
#include <string>

#include "gmock/gmock.h"
#include "google/api/distribution.pb.h"
#include "google/api/metric.pb.h"
#include "google/monitoring/v3/common.pb.h"
#include "google/monitoring/v3/metric.pb.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {
namespace testing {

bool TimeSeriesMatcher::MatchAndExplain(
    google::monitoring::v3::TimeSeries time_series,
    ::testing::MatchResultListener* listener) const {
  // Check labels.
  const auto& labels = time_series.metric().labels();
  for (const auto& label : labels) {
    if (tags_.find(label.first) == tags_.end()) {
      *listener << "has an extra label \"" << label.first << "\"";
      return false;
    }
  }
  for (const auto& tag : tags_) {
    const auto& label = labels.find(tag.first);
    if (label == labels.end()) {
      *listener << "is missing the label \"" << tag.first << "\"";
      return false;
    }
    if (absl::string_view(label->second) != tag.second) {
      *listener << "has the wrong value for the label \"" << tag.first
                << "\": actual \"" << label->second << "\"";
      return false;
    }
  }

  // Check data.
  if (time_series.points_size() != 1) {
    *listener << "has " << time_series.points_size() << " points; expected 1";
    return false;
  }
  const auto& point = time_series.points(0);
  switch (type_) {
    case Type::kDouble: {
      // Note that proto3 does not allow distinguishing between default and
      // unset oneof cases.
      if (point.value().double_value() != double_value_) {
        *listener << "value is " << point.value().double_value()
                  << "; expected " << double_value_;
        return false;
      }
      return true;
    }
    case Type::kInt64: {
      if (point.value().int64_value() != int_value_) {
        *listener << "value is " << point.value().int64_value() << "; expected "
                  << int_value_;
        return false;
      }
      return true;
    }
    case Type::kDistribution: {
      const auto& distribution = point.value().distribution_value();
      if (distribution.count() != distribution_value_.count()) {
        *listener << "count is " << distribution.count() << "; expected "
                  << distribution_value_.count();
        return false;
      }
      if (distribution.mean() != distribution_value_.mean()) {
        *listener << "mean is " << distribution.mean() << "; expected "
                  << distribution_value_.mean();
        return false;
      }
      if (distribution.sum_of_squared_deviation() !=
          distribution_value_.sum_of_squared_deviation()) {
        *listener << "sum_of_squared_deviation is "
                  << distribution.sum_of_squared_deviation() << "; expected "
                  << distribution_value_.sum_of_squared_deviation();
        return false;
      }
      const auto& buckets = distribution.bucket_options().explicit_buckets();
      if (buckets.bounds_size() !=
          distribution_value_.bucket_boundaries().lower_boundaries().size()) {
        *listener << "has " << buckets.bounds_size() << " buckets; expected "
                  << distribution_value_.bucket_boundaries()
                         .lower_boundaries()
                         .size();
        return false;
      }
      for (int i = 0; i < buckets.bounds_size(); ++i) {
        if (buckets.bounds(i) !=
            distribution_value_.bucket_boundaries().lower_boundaries()[i]) {
          *listener << "has the wrong boundaries";
          return false;
        }
        if (distribution.bucket_counts(i) !=
            distribution_value_.bucket_counts()[i]) {
          *listener << "has " << distribution.bucket_counts(i)
                    << "values in the " << i << "th bucket; expected "
                    << distribution_value_.bucket_counts()[i];
          return false;
        }
      }
      return true;
    }
  }
}

void TimeSeriesMatcher::DescribeTo(::std::ostream* os) const {
  switch (type_) {
    case Type::kDouble: {
      *os << double_value_;
      break;
    }
    case Type::kInt64: {
      *os << int_value_;
      break;
    }
    case Type::kDistribution: {
      *os << distribution_value_.DebugString();
      break;
    }
  }
}

}  // namespace testing
}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
