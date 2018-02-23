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

#ifndef OPENCENSUS_EXPORTERS_STATS_INTERNAL_TESTING_TIME_SERIES_MATCHER_H_
#define OPENCENSUS_EXPORTERS_STATS_INTERNAL_TESTING_TIME_SERIES_MATCHER_H_

#include <cstdint>
#include <map>
#include <string>

#include "gmock/gmock.h"
#include "google/monitoring/v3/metric.pb.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {
namespace testing {

// A gMock matcher that checks the labels and value of a TimeSeries against the
// provided values. It assumes that the TimeSeries only contains one point, and
// does not check timestamps. Works with ElementsAre and related matchers.
// Example usage:
//   std::vector<TimeSeries> ts = ...;
//   EXPECT_THAT(ts, ::testing::UnorderedElementsAre(
//                       testing::TimeSeriesDouble(
//                           {{"k1", "v1"}, {"k2", "v1"}}, 1.0),
//                       testing::TimeSeriesDouble(
//                           {{"k1", "v1"}, {"k2", "v2"}}, 2.0)));
class TimeSeriesMatcher
    : public ::testing::MatcherInterface<google::monitoring::v3::TimeSeries> {
 public:
  TimeSeriesMatcher(const std::map<std::string, std::string>& tags,
                    double value)
      : tags_(tags), type_(Type::kDouble), double_value_(value) {}
  TimeSeriesMatcher(const std::map<std::string, std::string>& tags,
                    int64_t value)
      : tags_(tags), type_(Type::kInt64), int_value_(value) {}
  TimeSeriesMatcher(const std::map<std::string, std::string>& tags,
                    const opencensus::stats::Distribution& value)
      : tags_(tags), type_(Type::kDistribution), distribution_value_(value) {}

  ~TimeSeriesMatcher() override {
    if (type_ == Type::kDistribution) {
      distribution_value_.~Distribution();
    }
  }

  bool MatchAndExplain(google::monitoring::v3::TimeSeries time_series,
                       ::testing::MatchResultListener* listener) const override;

  void DescribeTo(::std::ostream* os) const override;

 private:
  enum class Type {
    kDouble,
    kInt64,
    kDistribution,
  };

  const std::map<std::string, std::string> tags_;
  Type type_;
  union {
    const double double_value_;
    const int64_t int_value_;
    const opencensus::stats::Distribution distribution_value_;
  };
};

inline ::testing::Matcher<google::monitoring::v3::TimeSeries> TimeSeriesDouble(
    const std::map<std::string, std::string>& tags, double value) {
  return MakeMatcher(new TimeSeriesMatcher(tags, static_cast<double>(value)));
}
inline ::testing::Matcher<google::monitoring::v3::TimeSeries> TimeSeriesInt(
    const std::map<std::string, std::string>& tags, int64_t value) {
  return MakeMatcher(new TimeSeriesMatcher(tags, static_cast<int64_t>(value)));
}
inline ::testing::Matcher<google::monitoring::v3::TimeSeries>
TimeSeriesDistribution(const std::map<std::string, std::string>& tags,
                       const opencensus::stats::Distribution& value) {
  return MakeMatcher(new TimeSeriesMatcher(tags, value));
}

}  // namespace testing
}  // namespace stats
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_STATS_INTERNAL_TESTING_TIME_SERIES_MATCHER_H_
