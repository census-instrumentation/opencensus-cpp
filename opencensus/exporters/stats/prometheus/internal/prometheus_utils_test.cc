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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "metrics.pb.h"
#include "opencensus/stats/stats.h"
#include "opencensus/stats/testing/test_utils.h"

using opencensus::stats::testing::TestUtils;

namespace opencensus {
namespace exporters {
namespace stats {
namespace {

// Parses 'expected' as a text MetricFamily proto and tests that 'actual' equals
// 'expected', ignoring metric/label order.
void CompareMetricFamilies(const io::prometheus::client::MetricFamily& actual,
                           const std::string& expected_string) {
  io::prometheus::client::MetricFamily expected;
  google::protobuf::TextFormat::ParseFromString(expected_string, &expected);
  google::protobuf::util::MessageDifferencer differencer;
  const auto* metric_field_descriptor =
      expected.GetDescriptor()->FindFieldByName("metric");
  differencer.TreatAsSet(metric_field_descriptor);
  differencer.TreatAsSet(
      metric_field_descriptor->message_type()->FindFieldByName("label"));
  std::string output;
  differencer.ReportDifferencesToString(&output);
  EXPECT_TRUE(differencer.Compare(expected, actual)) << output;
}

TEST(SetMetricFamilyTest, CountDouble) {
  const auto measure = opencensus::stats::MeasureRegistry::RegisterDouble(
      "measure_count_double", "units", "");
  const std::string task = "test_task";
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::stats::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::stats::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Count())
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data = TestUtils::MakeViewData(
      view_descriptor,
      {{{"v1", "v1"}, 1.0}, {{"v1", "v1"}, 3.0}, {{"v1", "v2"}, 2.0}});
  io::prometheus::client::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  CompareMetricFamilies(actual, R"(
      name: "test_descriptor_units"
      type: COUNTER
      metric {
        label { name: "foo" value: "v1" }
        label { name: "bar" value: "v1" }
        counter { value: 2 }
      }
      metric {
        label { name: "foo" value: "v1" }
        label { name: "bar" value: "v2" }
        counter { value: 1 }
      }
  )");
}

TEST(SetMetricFamilyTest, SumDouble) {
  const auto measure = opencensus::stats::MeasureRegistry::RegisterDouble(
      "measure_sum_double", "units", "");
  const std::string task = "test_task";
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::stats::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::stats::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data = TestUtils::MakeViewData(
      view_descriptor,
      {{{"v1", "v1"}, 1.0}, {{"v1", "v1"}, 3.0}, {{"v1", "v2"}, 2.0}});
  io::prometheus::client::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  CompareMetricFamilies(actual, R"(
      name: "test_descriptor_units"
      type: UNTYPED
      metric {
        label { name: "foo" value: "v1" }
        label { name: "bar" value: "v1" }
        untyped { value: 4 }
      }
      metric {
        label { name: "foo" value: "v1" }
        label { name: "bar" value: "v2" }
        untyped { value: 2 }
      }
  )");
}

TEST(SetMetricFamilyTest, SumInt) {
  const auto measure = opencensus::stats::MeasureRegistry::RegisterInt(
      "measure_sum_int", "units", "");
  const std::string task = "test_task";
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::stats::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::stats::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data = TestUtils::MakeViewData(
      view_descriptor,
      {{{"v1", "v1"}, 1}, {{"v1", "v1"}, 3}, {{"v1", "v2"}, 2}});
  io::prometheus::client::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  CompareMetricFamilies(actual, R"(
      name: "test_descriptor_units"
      type: UNTYPED
      metric {
        label { name: "bar" value: "v1" }
        label { name: "foo" value: "v1" }
        untyped { value: 4 }
      }
      metric {
        label { name: "bar" value: "v2" }
        label { name: "foo" value: "v1" }
        untyped { value: 2 }
      }
  )");
}

TEST(StackdriverUtilsTest, MakeTimeSeriesDistributionDouble) {
  const auto measure = opencensus::stats::MeasureRegistry::RegisterDouble(
      "measure_distribution_double", "units", "");
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::stats::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::stats::TagKey::Register("bar");
  const auto bucket_boundaries =
      opencensus::stats::BucketBoundaries::Explicit({0, 10});
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(
              opencensus::stats::Aggregation::Distribution(bucket_boundaries))
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data =
      TestUtils::MakeViewData(view_descriptor, {{{"v1", "v1"}, -1.0},
                                                {{"v1", "v1"}, 7.0},
                                                {{"v1", "v2"}, 1.0},
                                                {{"v1", "v2"}, 11.0}});
  io::prometheus::client::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  CompareMetricFamilies(actual, R"(
      name: "test_descriptor_units"
      type: HISTOGRAM
      metric {
        label { name: "foo" value: "v1" }
        label { name: "bar" value: "v1" }
        histogram {
          sample_count: 2
          sample_sum: 6.0
          bucket { cumulative_count: 1 upper_bound: 0 }
          bucket { cumulative_count: 2 upper_bound: 10 }
          bucket { cumulative_count: 2 upper_bound: Inf }
        }
      }
      metric {
        label { name: "foo" value: "v1" }
        label { name: "bar" value: "v2" }
        histogram {
          sample_count: 2
          sample_sum: 12.0
          bucket { cumulative_count: 0 upper_bound: 0 }
          bucket { cumulative_count: 1 upper_bound: 10 }
          bucket { cumulative_count: 2 upper_bound: Inf }
        }
      }
  )");
}

}  // namespace
}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
