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
#include <vector>

#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "google/api/distribution.pb.h"
#include "google/api/label.pb.h"
#include "google/api/metric.pb.h"
#include "google/api/monitored_resource.pb.h"
#include "google/monitoring/v3/common.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "gtest/gtest.h"
#include "opencensus/exporters/stats/stackdriver/internal/time_series_matcher.h"
#include "opencensus/stats/stats.h"
#include "opencensus/stats/testing/test_utils.h"

using opencensus::stats::testing::TestUtils;

namespace opencensus {
namespace exporters {
namespace stats {
namespace {

TEST(StackdriverUtilsTest, SetMetricDescriptorNameAndType) {
  const std::string project_id = "projects/test-id";
  const std::string metric_name_prefix = "custom.googleapis.com/test/";
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_name("example.com/metric_name");
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor(project_id, metric_name_prefix, view_descriptor,
                      &metric_descriptor);

  const std::string expected_type =
      "custom.googleapis.com/test/example.com/metric_name";
  const std::string expected_name =
      absl::StrCat("projects/test-id/metricDescriptors/", expected_type);
  EXPECT_EQ(expected_name, metric_descriptor.name());
  EXPECT_EQ(expected_type, metric_descriptor.type());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorLabels) {
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().add_column(tag_key_1).add_column(
          tag_key_2);
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);

  ASSERT_EQ(3, metric_descriptor.labels_size());
  EXPECT_EQ("opencensus_task", metric_descriptor.labels(0).key());
  EXPECT_EQ(google::api::LabelDescriptor::STRING,
            metric_descriptor.labels(0).value_type());
  EXPECT_EQ(tag_key_1.name(), metric_descriptor.labels(1).key());
  EXPECT_EQ(google::api::LabelDescriptor::STRING,
            metric_descriptor.labels(1).value_type());
  EXPECT_EQ(tag_key_2.name(), metric_descriptor.labels(2).key());
  EXPECT_EQ(google::api::LabelDescriptor::STRING,
            metric_descriptor.labels(2).value_type());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorMetricKind) {
  auto view_descriptor = opencensus::stats::ViewDescriptor();
  google::api::MetricDescriptor metric_descriptor;

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Count());
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::CUMULATIVE,
            metric_descriptor.metric_kind());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Sum());
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::CUMULATIVE,
            metric_descriptor.metric_kind());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::LastValue());
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::GAUGE,
            metric_descriptor.metric_kind());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Distribution(
      opencensus::stats::BucketBoundaries::Explicit({})));
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::CUMULATIVE,
            metric_descriptor.metric_kind());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorValueType) {
  auto view_descriptor = opencensus::stats::ViewDescriptor();
  google::api::MetricDescriptor metric_descriptor;
  opencensus::stats::MeasureDouble::Register("double_measure", "", "");
  opencensus::stats::MeasureInt64::Register("int_measure", "", "");

  // Sum depends on measure type.
  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Sum());
  view_descriptor.set_measure("double_measure");
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::DOUBLE,
            metric_descriptor.value_type());

  view_descriptor.set_measure("int_measure");
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::INT64,
            metric_descriptor.value_type());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Count());
  view_descriptor.set_measure("double_measure");
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::INT64,
            metric_descriptor.value_type());
  view_descriptor.set_measure("int_measure");
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::INT64,
            metric_descriptor.value_type());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Distribution(
      opencensus::stats::BucketBoundaries::Explicit({0})));
  view_descriptor.set_measure("double_measure");
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::DISTRIBUTION,
            metric_descriptor.value_type());
  view_descriptor.set_measure("int_measure");
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::DISTRIBUTION,
            metric_descriptor.value_type());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorUnits) {
  const std::string units = "test_units";
  opencensus::stats::MeasureDouble::Register("measure", "", units);
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_measure("measure");
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);

  EXPECT_EQ(units, metric_descriptor.unit());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorUnitsCount) {
  opencensus::stats::MeasureDouble::Register("measure", "", "test_units");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_measure("measure")
          .set_aggregation(opencensus::stats::Aggregation::Count());
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);

  EXPECT_EQ("1", metric_descriptor.unit());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorDescription) {
  const std::string description = "test description";
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_description(description);
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor("", "", view_descriptor, &metric_descriptor);

  EXPECT_EQ(description, metric_descriptor.description());
}

TEST(StackdriverUtilsTest, MakeTimeSeriesSumDoubleAndTypes) {
  const std::string metric_name_prefix = "custom.googleapis.com/test/";
  const auto measure =
      opencensus::stats::MeasureDouble::Register("measure_sum_double", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_view";
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data = TestUtils::MakeViewData(
      view_descriptor, {{{"v1", "v1"}, 1.0}, {{"v1", "v2"}, 2.0}});
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries(metric_name_prefix, view_descriptor, data, task);

  for (const auto& ts : time_series) {
    EXPECT_EQ("custom.googleapis.com/test/test_view", ts.metric().type());
    EXPECT_EQ("global", ts.resource().type());
    ASSERT_EQ(1, ts.points_size());
    EXPECT_EQ(absl::ToUnixSeconds(data.start_time()),
              ts.points(0).interval().start_time().seconds());
    EXPECT_EQ(absl::ToUnixSeconds(data.end_time()),
              ts.points(0).interval().end_time().seconds());
  }

  EXPECT_THAT(time_series,
              ::testing::UnorderedElementsAre(
                  testing::TimeSeriesDouble({{"opencensus_task", task},
                                             {tag_key_1.name(), "v1"},
                                             {tag_key_2.name(), "v1"}},
                                            1.0),
                  testing::TimeSeriesDouble({{"opencensus_task", task},
                                             {tag_key_1.name(), "v1"},
                                             {tag_key_2.name(), "v2"}},
                                            2.0)));
}

TEST(StackdriverUtilsTest, MakeTimeSeriesSumInt) {
  const auto measure =
      opencensus::stats::MeasureInt64::Register("measure_sum_int", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data = TestUtils::MakeViewData(
      view_descriptor, {{{"v1", "v1"}, 1.0}, {{"v1", "v2"}, 2.0}});
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries("", view_descriptor, data, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    EXPECT_EQ(absl::ToUnixSeconds(data.start_time()),
              ts.points(0).interval().start_time().seconds());
    EXPECT_EQ(absl::ToUnixSeconds(data.end_time()),
              ts.points(0).interval().end_time().seconds());
  }

  EXPECT_THAT(time_series,
              ::testing::UnorderedElementsAre(
                  testing::TimeSeriesInt({{"opencensus_task", task},
                                          {tag_key_1.name(), "v1"},
                                          {tag_key_2.name(), "v1"}},
                                         1),
                  testing::TimeSeriesInt({{"opencensus_task", task},
                                          {tag_key_1.name(), "v1"},
                                          {tag_key_2.name(), "v2"}},
                                         2)));
}

TEST(StackdriverUtilsTest, MakeTimeSeriesCountDouble) {
  const auto measure = opencensus::stats::MeasureDouble::Register(
      "measure_count_double", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
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
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries("", view_descriptor, data, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    EXPECT_EQ(absl::ToUnixSeconds(data.start_time()),
              ts.points(0).interval().start_time().seconds());
    EXPECT_EQ(absl::ToUnixSeconds(data.end_time()),
              ts.points(0).interval().end_time().seconds());
  }

  EXPECT_THAT(time_series,
              ::testing::UnorderedElementsAre(
                  testing::TimeSeriesInt({{"opencensus_task", task},
                                          {tag_key_1.name(), "v1"},
                                          {tag_key_2.name(), "v1"}},
                                         2),
                  testing::TimeSeriesInt({{"opencensus_task", task},
                                          {tag_key_1.name(), "v1"},
                                          {tag_key_2.name(), "v2"}},
                                         1)));
}

TEST(StackdriverUtilsTest, MakeTimeSeriesDistributionDouble) {
  const auto measure = opencensus::stats::MeasureDouble::Register(
      "measure_distribution_double", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_view";
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
  const auto bucket_boundaries =
      opencensus::stats::BucketBoundaries::Explicit({0});
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(
              opencensus::stats::Aggregation::Distribution(bucket_boundaries))
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data = TestUtils::MakeViewData(
      view_descriptor,
      {{{"v1", "v1"}, -1.0}, {{"v1", "v1"}, 7.0}, {{"v1", "v2"}, 1.0}});
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries("", view_descriptor, data, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    EXPECT_EQ(absl::ToUnixSeconds(data.start_time()),
              ts.points(0).interval().start_time().seconds());
    EXPECT_EQ(absl::ToUnixSeconds(data.end_time()),
              ts.points(0).interval().end_time().seconds());
  }

  auto distribution1 = TestUtils::MakeDistribution(&bucket_boundaries);
  TestUtils::AddToDistribution(&distribution1, -1.0);
  TestUtils::AddToDistribution(&distribution1, 7.0);

  auto distribution2 = TestUtils::MakeDistribution(&bucket_boundaries);
  TestUtils::AddToDistribution(&distribution2, 1.0);

  EXPECT_THAT(time_series,
              ::testing::UnorderedElementsAre(
                  testing::TimeSeriesDistribution({{"opencensus_task", task},
                                                   {tag_key_1.name(), "v1"},
                                                   {tag_key_2.name(), "v1"}},
                                                  distribution1),
                  testing::TimeSeriesDistribution({{"opencensus_task", task},
                                                   {tag_key_1.name(), "v1"},
                                                   {tag_key_2.name(), "v2"}},
                                                  distribution2)));
}

TEST(StackdriverUtilsTest, MakeTimeSeriesLastValueInt) {
  const auto measure = opencensus::stats::MeasureInt64::Register(
      "measure_last_value_int", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::LastValue())
          .add_column(tag_key_1)
          .add_column(tag_key_2);
  const opencensus::stats::ViewData data = TestUtils::MakeViewData(
      view_descriptor, {{{"v1", "v1"}, 1.0}, {{"v1", "v2"}, 2.0}});
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries("", view_descriptor, data, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    EXPECT_FALSE(ts.points(0).interval().has_start_time());
    EXPECT_EQ(absl::ToUnixSeconds(data.end_time()),
              ts.points(0).interval().end_time().seconds());
  }

  EXPECT_THAT(time_series,
              ::testing::UnorderedElementsAre(
                  testing::TimeSeriesInt({{"opencensus_task", task},
                                          {tag_key_1.name(), "v1"},
                                          {tag_key_2.name(), "v1"}},
                                         1),
                  testing::TimeSeriesInt({{"opencensus_task", task},
                                          {tag_key_1.name(), "v1"},
                                          {tag_key_2.name(), "v2"}},
                                         2)));
}

}  // namespace
}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
