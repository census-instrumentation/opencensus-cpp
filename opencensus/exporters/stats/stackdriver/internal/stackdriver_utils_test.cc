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
using opencensus::stats::testing::TestViewValue;

namespace opencensus {
namespace exporters {
namespace stats {
namespace {

ABSL_CONST_INIT const absl::string_view kProjectName = "projects/test-project";
ABSL_CONST_INIT const absl::string_view kMetricNamePrefix =
    "custom.googleapis.com/test/";

// Example built-in metric from:
// https://cloud.google.com/monitoring/api/metrics_gcp
ABSL_CONST_INIT const absl::string_view kBuiltinMetricNamePrefix =
    "bigquery.googleapis.com/query/";

constexpr bool kAddTaskLabel = true;
constexpr bool kDoNotAddTaskLabel = false;

const google::api::MonitoredResource* kDefaultResource = nullptr;

// Helper that returns an example gce_instance resource.
google::api::MonitoredResource GceResource() {
  google::api::MonitoredResource resource;
  resource.set_type("gce_instance");
  (*resource.mutable_labels())["project_id"] = "my_project";
  (*resource.mutable_labels())["instance_id"] = "1234";
  (*resource.mutable_labels())["zone"] = "my_zone";
  return resource;
}

// Helper that returns an empty map of per-metric resources.
std::unordered_map<std::string, google::api::MonitoredResource>
DefaultPerMetricResource() {
  return std::unordered_map<std::string, google::api::MonitoredResource>();
}

std::vector<std::string> ToDataMapKeys(
    const std::initializer_list<std::string> label_names,
    const google::monitoring::v3::TimeSeries& time_series) {
  // label_names defines the key order and is needed because
  // time_series.metric().labels() does not maintain the original order.
  std::vector<std::string> data_map_keys;
  const auto& label_map = time_series.metric().labels();
  for (const auto& label_name : label_names) {
    data_map_keys.push_back(label_map.at(label_name));
  }
  return data_map_keys;
}

// TODO: MonitoredResourceForView
TEST(StackdriverUtilsTest, MonitoredResourceForViewIsDefault) {
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_name("example.com/metric_name");
  google::api::MonitoredResource empty;
  EXPECT_EQ(nullptr, MonitoredResourceForView(view_descriptor, empty,
                                              DefaultPerMetricResource()));
}

TEST(StackdriverUtilsTest, MonitoredResourceForViewIsCustomResource) {
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_name("example.com/metric_name");
  google::api::MonitoredResource monitored_resource = GceResource();
  EXPECT_EQ(&monitored_resource,
            MonitoredResourceForView(view_descriptor, monitored_resource,
                                     DefaultPerMetricResource()));
}

TEST(StackdriverUtilsTest, MonitoredResourceForViewIsPerViewResource) {
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_name("example.com/metric_name");
  google::api::MonitoredResource monitored_resource = GceResource();
  std::unordered_map<std::string, google::api::MonitoredResource> per_metric;
  per_metric["example.com/metric_name"].set_type("my_per_view_resource");
  const google::api::MonitoredResource* result =
      MonitoredResourceForView(view_descriptor, monitored_resource, per_metric);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ("my_per_view_resource", result->type());
}

TEST(StackdriverUtilsTest, MonitoredResourceForViewNoMatch) {
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_name("example.com/metric_name");
  google::api::MonitoredResource monitored_resource = GceResource();
  std::unordered_map<std::string, google::api::MonitoredResource> per_metric;
  per_metric["example.com/mismatched_metric_name"].set_type(
      "my_per_view_resource");
  EXPECT_EQ(&monitored_resource,
            MonitoredResourceForView(view_descriptor, monitored_resource,
                                     per_metric));
}

TEST(StackdriverUtilsTest, SetMetricDescriptorNameAndType) {
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_name("example.com/metric_name");
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(
      "projects/test-project/metricDescriptors/custom.googleapis.com/test/"
      "example.com/metric_name",
      metric_descriptor.name());
  EXPECT_EQ("custom.googleapis.com/test/example.com/metric_name",
            metric_descriptor.type());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorLabels) {
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().add_column(tag_key_1).add_column(
          tag_key_2);
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kAddTaskLabel, &metric_descriptor);

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

TEST(StackdriverUtilsTest, SetMetricDescriptorNoTaskLabel) {
  const auto tag_key = opencensus::tags::TagKey::Register("key");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().add_column(tag_key);
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor(kProjectName, kBuiltinMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);

  ASSERT_EQ(1, metric_descriptor.labels_size())
      << "Expect no opencensus_task label.";
  EXPECT_EQ(tag_key.name(), metric_descriptor.labels(0).key());
  EXPECT_EQ(google::api::LabelDescriptor::STRING,
            metric_descriptor.labels(0).value_type());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorMetricKind) {
  auto view_descriptor = opencensus::stats::ViewDescriptor();
  google::api::MetricDescriptor metric_descriptor;

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Count());
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::CUMULATIVE,
            metric_descriptor.metric_kind());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Sum());
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::CUMULATIVE,
            metric_descriptor.metric_kind());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::LastValue());
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::GAUGE,
            metric_descriptor.metric_kind());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Distribution(
      opencensus::stats::BucketBoundaries::Explicit({})));
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
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
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::DOUBLE,
            metric_descriptor.value_type());

  view_descriptor.set_measure("int_measure");
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::INT64,
            metric_descriptor.value_type());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Count());
  view_descriptor.set_measure("double_measure");
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::INT64,
            metric_descriptor.value_type());
  view_descriptor.set_measure("int_measure");
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::INT64,
            metric_descriptor.value_type());

  view_descriptor.set_aggregation(opencensus::stats::Aggregation::Distribution(
      opencensus::stats::BucketBoundaries::Explicit({0})));
  view_descriptor.set_measure("double_measure");
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::DISTRIBUTION,
            metric_descriptor.value_type());
  view_descriptor.set_measure("int_measure");
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);
  EXPECT_EQ(google::api::MetricDescriptor::DISTRIBUTION,
            metric_descriptor.value_type());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorUnits) {
  const std::string units = "test_units";
  opencensus::stats::MeasureDouble::Register("measure", "", units);
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_measure("measure");
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);

  EXPECT_EQ(units, metric_descriptor.unit());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorUnitsCount) {
  opencensus::stats::MeasureDouble::Register("measure", "", "test_units");
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_measure("measure")
          .set_aggregation(opencensus::stats::Aggregation::Count());
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);

  EXPECT_EQ("1", metric_descriptor.unit());
}

TEST(StackdriverUtilsTest, SetMetricDescriptorDescription) {
  const std::string description = "test description";
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor().set_description(description);
  google::api::MetricDescriptor metric_descriptor;
  SetMetricDescriptor(kProjectName, kMetricNamePrefix, view_descriptor,
                      kDoNotAddTaskLabel, &metric_descriptor);

  EXPECT_EQ(description, metric_descriptor.description());
}

TEST(StackdriverUtilsTest, MakeTimeSeriesDefaultResource) {
  const auto measure =
      opencensus::stats::MeasureDouble::Register("my_measure", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_view";
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Sum());
  const opencensus::stats::ViewData data =
      TestUtils::MakeViewData(view_descriptor, {{{}, 1.0}});
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries(kMetricNamePrefix, kDefaultResource, view_descriptor, data,
                     kDoNotAddTaskLabel, task);
  ASSERT_EQ(1, time_series.size());
  const auto& ts = time_series.front();
  EXPECT_EQ(0, ts.metric().labels_size()) << "There should be no task label.";
  EXPECT_EQ("global", ts.resource().type());
  EXPECT_EQ(0, ts.resource().labels_size());
}

TEST(StackdriverUtilsTest, MakeTimeSeriesDefaultResourceAndTaskLabel) {
  const auto measure =
      opencensus::stats::MeasureDouble::Register("my_measure", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_view";
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Sum());
  const opencensus::stats::ViewData data =
      TestUtils::MakeViewData(view_descriptor, {{{}, 1.0}});
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries(kMetricNamePrefix, kDefaultResource, view_descriptor, data,
                     kAddTaskLabel, task);
  ASSERT_EQ(1, time_series.size());
  const auto& ts = time_series.front();
  EXPECT_THAT(ts.metric().labels(),
              ::testing::Contains(::testing::Key("opencensus_task")));
  EXPECT_EQ("global", ts.resource().type());
  EXPECT_EQ(0, ts.resource().labels_size());
}

TEST(StackdriverUtilsTest, MakeTimeSeriesCustomResource) {
  const auto measure =
      opencensus::stats::MeasureDouble::Register("my_measure", "", "");
  const std::string task = "test_task";
  const std::string view_name = "test_view";
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(view_name)
          .set_measure(measure.GetDescriptor().name())
          .set_aggregation(opencensus::stats::Aggregation::Sum());
  const opencensus::stats::ViewData data =
      TestUtils::MakeViewData(view_descriptor, {{{}, 1.0}});
  google::api::MonitoredResource resource = GceResource();
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries(kMetricNamePrefix, &resource, view_descriptor, data,
                     kDoNotAddTaskLabel, task);
  ASSERT_EQ(1, time_series.size());
  const auto& ts = time_series.front();
  EXPECT_EQ("gce_instance", ts.resource().type());
  using ::testing::Pair;
  EXPECT_THAT(ts.resource().labels(),
              ::testing::UnorderedElementsAre(Pair("project_id", "my_project"),
                                              Pair("instance_id", "1234"),
                                              Pair("zone", "my_zone")))
      << "  resource() is:\n"
      << ts.resource().DebugString();
}

TEST(StackdriverUtilsTest, MakeTimeSeriesSumDoubleAndTypes) {
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
      MakeTimeSeries(kMetricNamePrefix, kDefaultResource, view_descriptor, data,
                     kAddTaskLabel, task);

  for (const auto& ts : time_series) {
    EXPECT_EQ("custom.googleapis.com/test/test_view", ts.metric().type());
    ASSERT_EQ(1, ts.points_size());
    const auto& tags = ToDataMapKeys({"foo", "bar"}, ts);
    EXPECT_EQ(absl::ToUnixSeconds(data.start_times().at(tags)),
              ts.points(0).interval().start_time().seconds());
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
  absl::Time t1 = absl::UnixEpoch();
  absl::Time t2 = t1 + absl::Seconds(15);
  std::map<std::vector<std::string>, absl::Time> start_times{
      {{"v1", "v1"}, t1},
      {{"v1", "v2"}, t2},
  };

  std::vector<TestViewValue> view_values;
  TestViewValue view_value1;
  view_value1.tag_values = {"v1", "v1"};
  view_value1.value = 1.0;
  view_value1.start_time = t1;
  view_values.push_back(view_value1);

  TestViewValue view_value2;
  view_value2.tag_values = {"v1", "v2"};
  view_value2.value = 2.0;
  view_value2.start_time = t2;
  view_values.push_back(view_value2);

  const opencensus::stats::ViewData data =
      TestUtils::MakeViewDataWithStartTimes(view_descriptor, view_values);
  const std::vector<google::monitoring::v3::TimeSeries> time_series =
      MakeTimeSeries(kMetricNamePrefix, kDefaultResource, view_descriptor, data,
                     kAddTaskLabel, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    const auto& tags = ToDataMapKeys({"foo", "bar"}, ts);
    EXPECT_EQ(absl::ToUnixSeconds(start_times[tags]),
              ts.points(0).interval().start_time().seconds());
    EXPECT_EQ(absl::ToUnixSeconds(data.end_time()),
              ts.points(0).interval().end_time().seconds());
  }
  EXPECT_EQ(absl::ToUnixSeconds(data.start_time()),
            absl::ToUnixSeconds(absl::UnixEpoch()));

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
      MakeTimeSeries(kMetricNamePrefix, kDefaultResource, view_descriptor, data,
                     kAddTaskLabel, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    const auto& tags = ToDataMapKeys({"foo", "bar"}, ts);
    EXPECT_EQ(absl::ToUnixSeconds(data.start_times().at(tags)),
              ts.points(0).interval().start_time().seconds());
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
      MakeTimeSeries(kMetricNamePrefix, kDefaultResource, view_descriptor, data,
                     kAddTaskLabel, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    const auto& tags = ToDataMapKeys({"foo", "bar"}, ts);
    EXPECT_EQ(absl::ToUnixSeconds(data.start_times().at(tags)),
              ts.points(0).interval().start_time().seconds());
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
      MakeTimeSeries(kMetricNamePrefix, kDefaultResource, view_descriptor, data,
                     kAddTaskLabel, task);

  for (const auto& ts : time_series) {
    ASSERT_EQ(1, ts.points_size());
    const auto& tags = ToDataMapKeys({"foo", "bar"}, ts);
    EXPECT_EQ(absl::ToUnixSeconds(data.start_times().at(tags)),
              ts.points(0).interval().start_time().seconds());
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
