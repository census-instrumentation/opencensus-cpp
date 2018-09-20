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

#include <limits>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "opencensus/stats/stats.h"
#include "opencensus/stats/testing/test_utils.h"

using opencensus::stats::testing::TestUtils;

namespace opencensus {
namespace exporters {
namespace stats {
namespace {

TEST(SetMetricFamilyTest, CountDouble) {
  const auto measure = opencensus::stats::MeasureDouble::Register(
      "measure_count_double", "", "units");
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
  prometheus::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  EXPECT_THAT(
      actual,
      ::testing::AllOf(
          ::testing::Field(&prometheus::MetricFamily::name,
                           "test_descriptor_units"),
          ::testing::Field(&prometheus::MetricFamily::type,
                           prometheus::MetricType::Counter),
          ::testing::Field(
              &prometheus::MetricFamily::metric,
              ::testing::UnorderedElementsAre(
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::counter,
                          ::testing::Field(
                              &prometheus::ClientMetric::Counter::value, 2))),
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v2")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::counter,
                          ::testing::Field(
                              &prometheus::ClientMetric::Counter::value,
                              1)))))));
}

TEST(SetMetricFamilyTest, SumDouble) {
  const auto measure = opencensus::stats::MeasureDouble::Register(
      "measure_sum_double", "", "units");
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
      view_descriptor,
      {{{"v1", "v1"}, 1.0}, {{"v1", "v1"}, 3.0}, {{"v1", "v2"}, 2.0}});
  prometheus::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  EXPECT_THAT(
      actual,
      ::testing::AllOf(
          ::testing::Field(&prometheus::MetricFamily::name,
                           "test_descriptor_units"),
          ::testing::Field(&prometheus::MetricFamily::type,
                           prometheus::MetricType::Untyped),
          ::testing::Field(
              &prometheus::MetricFamily::metric,
              ::testing::UnorderedElementsAre(
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::untyped,
                          ::testing::Field(
                              &prometheus::ClientMetric::Untyped::value, 4))),
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v2")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::untyped,
                          ::testing::Field(
                              &prometheus::ClientMetric::Untyped::value,
                              2)))))));
}

TEST(SetMetricFamilyTest, SumInt) {
  const auto measure =
      opencensus::stats::MeasureInt64::Register("measure_sum_int", "", "units");
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
      view_descriptor,
      {{{"v1", "v1"}, 1}, {{"v1", "v1"}, 3}, {{"v1", "v2"}, 2}});
  prometheus::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  EXPECT_THAT(
      actual,
      ::testing::AllOf(
          ::testing::Field(&prometheus::MetricFamily::name,
                           "test_descriptor_units"),
          ::testing::Field(&prometheus::MetricFamily::type,
                           prometheus::MetricType::Untyped),
          ::testing::Field(
              &prometheus::MetricFamily::metric,
              ::testing::UnorderedElementsAre(
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::untyped,
                          ::testing::Field(
                              &prometheus::ClientMetric::Untyped::value, 4))),
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v2")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::untyped,
                          ::testing::Field(
                              &prometheus::ClientMetric::Untyped::value,
                              2)))))));
}

TEST(SetMetricFamilyTest, LastValueDouble) {
  const auto measure = opencensus::stats::MeasureDouble::Register(
      "measure_last_value_double", "", "units");
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
      view_descriptor,
      {{{"v1", "v1"}, 1.0}, {{"v1", "v1"}, 3.0}, {{"v1", "v2"}, 2.0}});
  prometheus::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  EXPECT_THAT(
      actual,
      ::testing::AllOf(
          ::testing::Field(&prometheus::MetricFamily::name,
                           "test_descriptor_units"),
          ::testing::Field(&prometheus::MetricFamily::type,
                           prometheus::MetricType::Gauge),
          ::testing::Field(
              &prometheus::MetricFamily::metric,
              ::testing::UnorderedElementsAre(
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::gauge,
                          ::testing::Field(
                              &prometheus::ClientMetric::Gauge::value, 3))),
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v2")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::gauge,
                          ::testing::Field(
                              &prometheus::ClientMetric::Gauge::value, 2)))))));
}

TEST(SetMetricFamilyTest, LastValueInt64) {
  const auto measure = opencensus::stats::MeasureInt64::Register(
      "measure_last_value_int", "", "units");
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
      view_descriptor,
      {{{"v1", "v1"}, 1}, {{"v1", "v1"}, 3}, {{"v1", "v2"}, 2}});
  prometheus::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  EXPECT_THAT(
      actual,
      ::testing::AllOf(
          ::testing::Field(&prometheus::MetricFamily::name,
                           "test_descriptor_units"),
          ::testing::Field(&prometheus::MetricFamily::type,
                           prometheus::MetricType::Gauge),
          ::testing::Field(
              &prometheus::MetricFamily::metric,
              ::testing::UnorderedElementsAre(
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::gauge,
                          ::testing::Field(
                              &prometheus::ClientMetric::Gauge::value, 3))),
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v2")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::gauge,
                          ::testing::Field(
                              &prometheus::ClientMetric::Gauge::value, 2)))))));
}

TEST(StackdriverUtilsTest, MakeTimeSeriesDistributionDouble) {
  const auto measure = opencensus::stats::MeasureDouble::Register(
      "measure_distribution_double", "", "units");
  const std::string view_name = "test_descriptor";
  const auto tag_key_1 = opencensus::tags::TagKey::Register("foo");
  const auto tag_key_2 = opencensus::tags::TagKey::Register("bar");
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
  prometheus::MetricFamily actual;
  SetMetricFamily(view_descriptor, data, &actual);

  EXPECT_THAT(
      actual,
      ::testing::AllOf(
          ::testing::Field(&prometheus::MetricFamily::name,
                           "test_descriptor_units"),
          ::testing::Field(&prometheus::MetricFamily::type,
                           prometheus::MetricType::Histogram),
          ::testing::Field(
              &prometheus::MetricFamily::metric,
              ::testing::UnorderedElementsAre(
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::histogram,
                          ::testing::AllOf(
                              ::testing::Field(&prometheus::ClientMetric::
                                                   Histogram::sample_count,
                                               2),
                              ::testing::Field(&prometheus::ClientMetric::
                                                   Histogram::sample_sum,
                                               6.0),
                              ::testing::Field(
                                  &prometheus::ClientMetric::Histogram::bucket,
                                  ::testing::ElementsAre(
                                      ::testing::AllOf(
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::cumulative_count,
                                              1),
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::upper_bound,
                                              0)),
                                      ::testing::AllOf(
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::cumulative_count,
                                              2),
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::upper_bound,
                                              10)),
                                      ::testing::AllOf(
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::cumulative_count,
                                              2),
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::upper_bound,
                                              std::numeric_limits<
                                                  double>::infinity()))))))),
                  ::testing::AllOf(
                      ::testing::Field(
                          &prometheus::ClientMetric::label,
                          ::testing::UnorderedElementsAre(
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "foo"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v1")),
                              ::testing::AllOf(
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::name,
                                      "bar"),
                                  ::testing::Field(
                                      &prometheus::ClientMetric::Label::value,
                                      "v2")))),
                      ::testing::Field(
                          &prometheus::ClientMetric::histogram,
                          ::testing::AllOf(
                              ::testing::Field(&prometheus::ClientMetric::
                                                   Histogram::sample_count,
                                               2),
                              ::testing::Field(&prometheus::ClientMetric::
                                                   Histogram::sample_sum,
                                               12.0),
                              ::testing::Field(
                                  &prometheus::ClientMetric::Histogram::bucket,
                                  ::testing::ElementsAre(
                                      ::testing::AllOf(
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::cumulative_count,
                                              0),
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::upper_bound,
                                              0)),
                                      ::testing::AllOf(
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::cumulative_count,
                                              1),
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::upper_bound,
                                              10)),
                                      ::testing::AllOf(
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::cumulative_count,
                                              2),
                                          ::testing::Field(
                                              &prometheus::ClientMetric::
                                                  Bucket::upper_bound,
                                              std::numeric_limits<double>::
                                                  infinity())))))))))));
}

}  // namespace
}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
