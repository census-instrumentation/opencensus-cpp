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

#include <cstdlib>
#include <iostream>

#include <grpcpp/grpcpp.h>
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "google/monitoring/v3/common.pb.h"
#include "google/monitoring/v3/metric_service.grpc.pb.h"
#include "google/protobuf/empty.pb.h"
#include "gtest/gtest.h"
#include "opencensus/exporters/stats/stackdriver/internal/stackdriver_utils.h"
#include "opencensus/exporters/stats/stackdriver/internal/time_series_matcher.h"
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {
namespace {

// This is a true end-to-end test for the Stackdriver Exporter, connecting to
// production Stackdriver. As such, it is subject to failures in networking or
// the Stackdriver backend; it also cannot be run multiple times simultaneously
// under the same Cloud project.
//
// See comments in stackdriver_exporter.h regarding setting up authentication.
// Additionally, it requires the environment variable STACKDRIVER_PROJECT_ID to
// be set to the project id corresponding to the GOOGLE_APPLICATION_CREDENTIALS,
// and for the account specified by the credentials to have the "Monitoring
// Viewer" permission.

constexpr char kGoogleStackdriverStatsAddress[] = "monitoring.googleapis.com";

constexpr char kTestMeasureName[] = "opencensus.io/TestMeasure";
opencensus::stats::MeasureDouble TestMeasure() {
  static const opencensus::stats::MeasureDouble foo_usage =
      opencensus::stats::MeasureDouble::Register(
          kTestMeasureName, "Test measure.", "1{test units}");
  return foo_usage;
}

class StackdriverE2eTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    TestMeasure();

    if (project_id_.empty() || credentials_.empty()) {
      std::cerr << "STACKDRIVER_PROJECT_ID or GOOGLE_APPLICATION_CREDENTIALS "
                   "unset.\n";
      std::abort();
    }
    StackdriverOptions opts;
    opts.project_id = std::string(project_id_);
    opts.opencensus_task = "test_task";
    StackdriverExporter::Register(opts);
  }

  // Retrieves data exported under 'descriptor'.
  std::vector<google::monitoring::v3::TimeSeries> RetrieveData(
      const opencensus::stats::ViewDescriptor& descriptor);

  // Cleans up by deleting the metric the Stackdriver exporter should have
  // created for 'descriptor'.
  void DeleteMetric(const opencensus::stats::ViewDescriptor& descriptor);

  static const absl::string_view project_id_;
  static const absl::string_view credentials_;
  // A prefix to add to view names because a newly-created Stackdriver metric
  // may resurrect data from deleted metrics with the same name.
  std::string prefix_ = absl::StrCat(absl::ToUnixMillis(absl::Now()));

  const std::unique_ptr<google::monitoring::v3::MetricService::Stub> stub_ =
      google::monitoring::v3::MetricService::NewStub(::grpc::CreateChannel(
          kGoogleStackdriverStatsAddress, ::grpc::GoogleDefaultCredentials()));

  const opencensus::tags::TagKey key1_ =
      opencensus::tags::TagKey::Register("key1");
  const opencensus::tags::TagKey key2_ =
      opencensus::tags::TagKey::Register("key2");
};

const absl::string_view StackdriverE2eTest::project_id_ =
    std::getenv("STACKDRIVER_PROJECT_ID");
const absl::string_view StackdriverE2eTest::credentials_ =
    std::getenv("GOOGLE_APPLICATION_CREDENTIALS");

std::vector<google::monitoring::v3::TimeSeries>
StackdriverE2eTest::RetrieveData(
    const opencensus::stats::ViewDescriptor& descriptor) {
  std::vector<google::monitoring::v3::TimeSeries> data;
  google::monitoring::v3::ListTimeSeriesRequest request;
  request.set_name(absl::StrCat("projects/", project_id_));
  request.set_filter(
      absl::StrCat("metric.type = \"custom.googleapis.com/opencensus/",
                   descriptor.name(), "\""));
  SetTimestamp(absl::Now() - absl::Hours(1),
               request.mutable_interval()->mutable_start_time());
  SetTimestamp(absl::Now() + absl::Hours(1),
               request.mutable_interval()->mutable_end_time());

  while (true) {
    google::monitoring::v3::ListTimeSeriesResponse response;
    ::grpc::ClientContext context;
    ::grpc::Status status = stub_->ListTimeSeries(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "ListTimeSeries error: " << status.error_message() << "\n";
      // This may mean that the data has not propagated through Stackdriver
      // yet.
      std::cout << "Waiting and retrying.\n";
      absl::SleepFor(absl::Seconds(5));
      continue;
    }

    // If there are too many response Stackdriver returns them in multiple
    // pages.
    for (const auto& ts : response.time_series()) {
      data.push_back(ts);
    }
    if (response.next_page_token().empty()) {
      break;
    }
    request.set_page_token(response.next_page_token());
  }
  return data;
}

void StackdriverE2eTest::DeleteMetric(
    const opencensus::stats::ViewDescriptor& descriptor) {
  // Remove from the exporter before deleting from Stackdriver so that the
  // exporter does not recreate it.
  opencensus::stats::StatsExporter::RemoveView(descriptor.name());

  google::monitoring::v3::DeleteMetricDescriptorRequest request;
  request.set_name(
      absl::StrCat("projects/", project_id_,
                   "/metricDescriptors/custom.googleapis.com/opencensus/",
                   descriptor.name()));
  ::grpc::ClientContext context;
  google::protobuf::Empty response;
  ::grpc::Status status =
      stub_->DeleteMetricDescriptor(&context, request, &response);
  EXPECT_TRUE(status.ok());
}

TEST_F(StackdriverE2eTest, OneView) {
  // Make sure that the simple case works.
  const auto view_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(absl::StrCat(
              "opencensus.io/Test/TestMeasure-sum-cumulative-key1-key2-",
              prefix_))
          .set_measure(kTestMeasureName)
          .set_aggregation(::opencensus::stats::Aggregation::Sum())
          .add_column(key1_)
          .add_column(key2_)
          .set_description(
              "Cumulative sum of opencensus.io/TestMeasure broken down "
              "by 'key1' and 'key2'.");
  view_descriptor.RegisterForExport();

  opencensus::stats::Record({{TestMeasure(), 1.0}},
                            {{key1_, "v11"}, {key2_, "v21"}});
  opencensus::stats::Record({{TestMeasure(), 2.0}},
                            {{key1_, "v11"}, {key2_, "v22"}});

  std::cout << "Waiting for data to propagate.\n";
  absl::SleepFor(absl::Seconds(40));
  EXPECT_THAT(RetrieveData(view_descriptor),
              ::testing::UnorderedElementsAre(
                  testing::TimeSeriesDouble({{"opencensus_task", "test_task"},
                                             {"key1", "v11"},
                                             {"key2", "v21"}},
                                            1.0),
                  testing::TimeSeriesDouble({{"opencensus_task", "test_task"},
                                             {"key1", "v11"},
                                             {"key2", "v22"}},
                                            2.0)));

  DeleteMetric(view_descriptor);
}

TEST_F(StackdriverE2eTest, LargeTest) {
  const auto count_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(absl::StrCat(
              "opencensus.io/Test/TestMeasure-count-cumulative-key1-key2-",
              prefix_))
          .set_measure(kTestMeasureName)
          .set_aggregation(::opencensus::stats::Aggregation::Count())
          .add_column(key1_)
          .add_column(key2_)
          .set_description(
              "Cumulative count of opencensus.io/TestMeasure broken down "
              "by 'key1' and 'key2'.");
  count_descriptor.RegisterForExport();

  const auto sum_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_name(absl::StrCat(
              "opencensus.io/Test/TestMeasure-sum-cumulative-key1-key2-",
              prefix_))
          .set_measure(kTestMeasureName)
          .set_aggregation(::opencensus::stats::Aggregation::Sum())
          .add_column(key1_)
          .add_column(key2_)
          .set_description(
              "Cumulative sum of opencensus.io/TestMeasure broken down "
              "by 'key1' and 'key2'.");
  sum_descriptor.RegisterForExport();

  std::vector<::testing::Matcher<google::monitoring::v3::TimeSeries>>
      sum_matchers;
  std::vector<::testing::Matcher<google::monitoring::v3::TimeSeries>>
      count_matchers;

  // The number of values to record for each tag--total metric cardinality will
  // be tag_values^2. We want this high enough that uploads require multiple
  // batches.
  const int tag_values = 25;
  for (int i = 0; i < tag_values; ++i) {
    for (int j = 0; j < tag_values; ++j) {
      const std::string tag1 = absl::StrCat("v1", i);
      const std::string tag2 = absl::StrCat("v1", j);
      const double value = i * j;
      opencensus::stats::Record({{TestMeasure(), value}},
                                {{key1_, tag1}, {key2_, tag2}});
      sum_matchers.push_back(testing::TimeSeriesDouble(
          {{"opencensus_task", "test_task"}, {"key1", tag1}, {"key2", tag2}},
          value));
      count_matchers.push_back(testing::TimeSeriesInt(
          {{"opencensus_task", "test_task"}, {"key1", tag1}, {"key2", tag2}},
          1));
    }
  }

  std::cout << "Waiting for data to propagate.\n";
  absl::SleepFor(absl::Seconds(40));

  const auto count_data = RetrieveData(count_descriptor);
  const auto sum_data = RetrieveData(sum_descriptor);
  ASSERT_EQ(tag_values * tag_values, count_data.size());
  ASSERT_EQ(tag_values * tag_values, sum_data.size());

  EXPECT_THAT(RetrieveData(count_descriptor),
              ::testing::UnorderedElementsAreArray(count_matchers));
  EXPECT_THAT(RetrieveData(sum_descriptor),
              ::testing::UnorderedElementsAreArray(sum_matchers));

  DeleteMetric(count_descriptor);
  DeleteMetric(sum_descriptor);
}

}  // namespace
}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
