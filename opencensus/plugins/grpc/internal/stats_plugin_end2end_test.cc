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

#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/grpc++/grpc++.h"
#include "opencensus/plugins/grpc/grpc_plugin.h"
#include "opencensus/plugins/grpc/internal/testing/echo.grpc.pb.h"
#include "opencensus/stats/stats.h"
#include "opencensus/stats/testing/test_utils.h"

namespace opencensus {
namespace testing {
namespace {

class EchoServer final : public EchoService::Service {
  ::grpc::Status Echo(::grpc::ServerContext* context,
                      const EchoRequest* request,
                      EchoResponse* response) override {
    if (request->status_code() == 0) {
      response->set_message(request->message());
      return ::grpc::Status::OK;
    } else {
      return ::grpc::Status(
          static_cast<::grpc::StatusCode>(request->status_code()), "");
    }
  }
};

class StatsPluginEnd2EndTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { RegisterGrpcPlugin(); }

  void SetUp() {
    // Set up a synchronous server on a different thread to avoid the asynch
    // interface.
    ::grpc::ServerBuilder builder;
    int port;
    // Use IPv4 here because it's less flaky than IPv6 ("[::]:0") on Travis.
    builder.AddListeningPort("0.0.0.0:0", ::grpc::InsecureServerCredentials(),
                             &port);
    builder.RegisterService(&service_);
    server_ = builder.BuildAndStart();
    ASSERT_NE(nullptr, server_);
    ASSERT_NE(0, port);
    server_address_ = absl::StrCat("0.0.0.0:", port);
    server_thread_ = std::thread(&StatsPluginEnd2EndTest::RunServerLoop, this);

    stub_ = EchoService::NewStub(::grpc::CreateChannel(
        server_address_, ::grpc::InsecureChannelCredentials()));
  }

  void TearDown() {
    server_->Shutdown();
    server_thread_.join();
  }

  void RunServerLoop() { server_->Wait(); }

  const std::string client_method_name_ =
      "Sent.opencensus.testing.EchoService/Echo";
  const std::string server_method_name_ =
      "Recv.opencensus.testing.EchoService/Echo";

  std::string server_address_;
  EchoServer service_;
  std::unique_ptr<grpc::Server> server_;
  std::thread server_thread_;

  std::unique_ptr<EchoService::Stub> stub_;
};

TEST_F(StatsPluginEnd2EndTest, ErrorCount) {
  const auto client_method_descriptor =
      stats::ViewDescriptor()
          .set_measure(kRpcClientErrorCountMeasureName)
          .set_name("client_method")
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(MethodTagKey());
  stats::View client_method_view(client_method_descriptor);
  const auto server_method_descriptor =
      stats::ViewDescriptor()
          .set_measure(kRpcServerErrorCountMeasureName)
          .set_name("server_method")
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(MethodTagKey());
  stats::View server_method_view(server_method_descriptor);

  const auto client_status_descriptor =
      stats::ViewDescriptor()
          .set_measure(kRpcClientErrorCountMeasureName)
          .set_name("client_status")
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(StatusTagKey());
  stats::View client_status_view(client_status_descriptor);
  const auto server_status_descriptor =
      stats::ViewDescriptor()
          .set_measure(kRpcServerErrorCountMeasureName)
          .set_name("server_status")
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(StatusTagKey());
  stats::View server_status_view(server_status_descriptor);

  // Cover all valid statuses.
  for (int i = 0; i <= 16; ++i) {
    EchoRequest request;
    request.set_message("foo");
    request.set_status_code(i);
    EchoResponse response;
    ::grpc::ClientContext context;
    ::grpc::Status status = stub_->Echo(&context, request, &response);
  }
  absl::SleepFor(absl::Milliseconds(500));
  stats::testing::TestUtils::Flush();

  EXPECT_THAT(client_method_view.GetData().double_data(),
              ::testing::UnorderedElementsAre(::testing::Pair(
                  ::testing::ElementsAre(client_method_name_), 16.0)));
  EXPECT_THAT(server_method_view.GetData().double_data(),
              ::testing::UnorderedElementsAre(::testing::Pair(
                  ::testing::ElementsAre(server_method_name_), 16.0)));

  auto codes = {
      ::testing::Pair(::testing::ElementsAre("OK"), 0.0),
      ::testing::Pair(::testing::ElementsAre("CANCELLED"), 1.0),
      ::testing::Pair(::testing::ElementsAre("UNKNOWN"), 1.0),
      ::testing::Pair(::testing::ElementsAre("INVALID_ARGUMENT"), 1.0),
      ::testing::Pair(::testing::ElementsAre("DEADLINE_EXCEEDED"), 1.0),
      ::testing::Pair(::testing::ElementsAre("NOT_FOUND"), 1.0),
      ::testing::Pair(::testing::ElementsAre("ALREADY_EXISTS"), 1.0),
      ::testing::Pair(::testing::ElementsAre("PERMISSION_DENIED"), 1.0),
      ::testing::Pair(::testing::ElementsAre("UNAUTHENTICATED"), 1.0),
      ::testing::Pair(::testing::ElementsAre("RESOURCE_EXHAUSTED"), 1.0),
      ::testing::Pair(::testing::ElementsAre("FAILED_PRECONDITION"), 1.0),
      ::testing::Pair(::testing::ElementsAre("ABORTED"), 1.0),
      ::testing::Pair(::testing::ElementsAre("OUT_OF_RANGE"), 1.0),
      ::testing::Pair(::testing::ElementsAre("UNIMPLEMENTED"), 1.0),
      ::testing::Pair(::testing::ElementsAre("INTERNAL"), 1.0),
      ::testing::Pair(::testing::ElementsAre("UNAVAILABLE"), 1.0),
      ::testing::Pair(::testing::ElementsAre("DATA_LOSS"), 1.0),
  };

  EXPECT_THAT(client_status_view.GetData().double_data(),
              ::testing::UnorderedElementsAreArray(codes));
  EXPECT_THAT(server_status_view.GetData().double_data(),
              ::testing::UnorderedElementsAreArray(codes));
}

TEST_F(StatsPluginEnd2EndTest, RequestResponseBytes) {
  stats::View client_request_bytes_view(ClientRequestBytesCumulative());
  stats::View client_response_bytes_view(ClientResponseBytesCumulative());
  stats::View server_request_bytes_view(ServerRequestBytesCumulative());
  stats::View server_response_bytes_view(ServerResponseBytesCumulative());

  {
    EchoRequest request;
    request.set_message("foo");
    EchoResponse response;
    ::grpc::ClientContext context;
    ::grpc::Status status = stub_->Echo(&context, request, &response);
    ASSERT_TRUE(status.ok());
    EXPECT_EQ("foo", response.message());
  }
  absl::SleepFor(absl::Milliseconds(500));
  stats::testing::TestUtils::Flush();

  EXPECT_THAT(
      client_request_bytes_view.GetData().distribution_data(),
      ::testing::UnorderedElementsAre(::testing::Pair(
          ::testing::ElementsAre(client_method_name_),
          ::testing::AllOf(::testing::Property(&stats::Distribution::count, 1),
                           ::testing::Property(&stats::Distribution::mean,
                                               ::testing::Gt(0.0))))));
  EXPECT_THAT(
      client_response_bytes_view.GetData().distribution_data(),
      ::testing::UnorderedElementsAre(::testing::Pair(
          ::testing::ElementsAre(client_method_name_),
          ::testing::AllOf(::testing::Property(&stats::Distribution::count, 1),
                           ::testing::Property(&stats::Distribution::mean,
                                               ::testing::Gt(0.0))))));
  EXPECT_THAT(
      server_request_bytes_view.GetData().distribution_data(),
      ::testing::UnorderedElementsAre(::testing::Pair(
          ::testing::ElementsAre(server_method_name_),
          ::testing::AllOf(::testing::Property(&stats::Distribution::count, 1),
                           ::testing::Property(&stats::Distribution::mean,
                                               ::testing::Gt(0.0))))));
  EXPECT_THAT(
      server_response_bytes_view.GetData().distribution_data(),
      ::testing::UnorderedElementsAre(::testing::Pair(
          ::testing::ElementsAre(server_method_name_),
          ::testing::AllOf(::testing::Property(&stats::Distribution::count, 1),
                           ::testing::Property(&stats::Distribution::mean,
                                               ::testing::Gt(0.0))))));
}

TEST_F(StatsPluginEnd2EndTest, Latency) {
  stats::View client_latency_view(ClientRoundtripLatencyCumulative());
  stats::View client_server_elapsed_time_view(
      ClientServerElapsedTimeCumulative());
  stats::View server_server_elapsed_time_view(
      ServerServerElapsedTimeCumulative());

  const absl::Time start_time = absl::Now();
  {
    EchoRequest request;
    request.set_message("foo");
    EchoResponse response;
    ::grpc::ClientContext context;
    ::grpc::Status status = stub_->Echo(&context, request, &response);
    ASSERT_TRUE(status.ok());
    EXPECT_EQ("foo", response.message());
  }
  // We do not know exact latency/elapsed time, but we know it is less than the
  // entire time spent making the RPC.
  const double max_time = absl::ToDoubleMilliseconds(absl::Now() - start_time);

  absl::SleepFor(absl::Milliseconds(500));
  stats::testing::TestUtils::Flush();

  EXPECT_THAT(
      client_latency_view.GetData().distribution_data(),
      ::testing::UnorderedElementsAre(::testing::Pair(
          ::testing::ElementsAre(client_method_name_),
          ::testing::AllOf(::testing::Property(&stats::Distribution::count, 1),
                           ::testing::Property(&stats::Distribution::mean,
                                               ::testing::Gt(0.0)),
                           ::testing::Property(&stats::Distribution::mean,
                                               ::testing::Lt(max_time))))));

  // Elapsed time is a subinterval of total latency.
  const auto client_latency = client_latency_view.GetData()
                                  .distribution_data()
                                  .find({client_method_name_})
                                  ->second.mean();
  EXPECT_THAT(client_server_elapsed_time_view.GetData().distribution_data(),
              ::testing::UnorderedElementsAre(::testing::Pair(
                  ::testing::ElementsAre(client_method_name_),
                  ::testing::AllOf(
                      ::testing::Property(&stats::Distribution::count, 1),
                      ::testing::Property(&stats::Distribution::mean,
                                          ::testing::Gt(0.0)),
                      ::testing::Property(&stats::Distribution::mean,
                                          ::testing::Lt(client_latency))))));

  // client server elapsed time should be the same value propagated to the
  // client.
  const auto client_elapsed_time = client_server_elapsed_time_view.GetData()
                                       .distribution_data()
                                       .find({client_method_name_})
                                       ->second.mean();
  EXPECT_THAT(
      server_server_elapsed_time_view.GetData().distribution_data(),
      ::testing::UnorderedElementsAre(::testing::Pair(
          ::testing::ElementsAre(server_method_name_),
          ::testing::AllOf(
              ::testing::Property(&stats::Distribution::count, 1),
              ::testing::Property(&stats::Distribution::mean,
                                  ::testing::DoubleEq(client_elapsed_time))))));
}

TEST_F(StatsPluginEnd2EndTest, StartFinishCount) {
  stats::View client_started_count_view(ClientStartedCountCumulative());
  stats::View client_finished_count_view(ClientFinishedCountCumulative());
  stats::View server_started_count_view(ServerStartedCountCumulative());
  stats::View server_finished_count_view(ServerFinishedCountCumulative());

  EchoRequest request;
  request.set_message("foo");
  EchoResponse response;
  const int count = 5;
  for (int i = 0; i < count; ++i) {
    {
      ::grpc::ClientContext context;
      ::grpc::Status status = stub_->Echo(&context, request, &response);
      ASSERT_TRUE(status.ok());
      EXPECT_EQ("foo", response.message());
    }
    absl::SleepFor(absl::Milliseconds(500));
    stats::testing::TestUtils::Flush();

    EXPECT_THAT(client_started_count_view.GetData().double_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(client_method_name_), i + 1)));
    EXPECT_THAT(client_finished_count_view.GetData().double_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(client_method_name_), i + 1)));
    EXPECT_THAT(server_started_count_view.GetData().double_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(server_method_name_), i + 1)));
    EXPECT_THAT(server_finished_count_view.GetData().double_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(server_method_name_), i + 1)));
  }
}

TEST_F(StatsPluginEnd2EndTest, RequestResponseCount) {
  // TODO: Use streaming RPCs.
  stats::View client_request_count_view(ClientRequestCountCumulative());
  stats::View client_response_count_view(ClientResponseCountCumulative());
  stats::View server_request_count_view(ServerRequestCountCumulative());
  stats::View server_response_count_view(ServerResponseCountCumulative());

  EchoRequest request;
  request.set_message("foo");
  EchoResponse response;
  const int count = 5;
  for (int i = 0; i < count; ++i) {
    {
      ::grpc::ClientContext context;
      ::grpc::Status status = stub_->Echo(&context, request, &response);
      ASSERT_TRUE(status.ok());
      EXPECT_EQ("foo", response.message());
    }
    absl::SleepFor(absl::Milliseconds(500));
    stats::testing::TestUtils::Flush();

    EXPECT_THAT(client_request_count_view.GetData().distribution_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(client_method_name_),
                    ::testing::AllOf(
                        ::testing::Property(&stats::Distribution::count, i + 1),
                        ::testing::Property(&stats::Distribution::mean,
                                            ::testing::DoubleEq(1.0))))));
    EXPECT_THAT(client_response_count_view.GetData().distribution_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(client_method_name_),
                    ::testing::AllOf(
                        ::testing::Property(&stats::Distribution::count, i + 1),
                        ::testing::Property(&stats::Distribution::mean,
                                            ::testing::DoubleEq(1.0))))));
    EXPECT_THAT(server_request_count_view.GetData().distribution_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(server_method_name_),
                    ::testing::AllOf(
                        ::testing::Property(&stats::Distribution::count, i + 1),
                        ::testing::Property(&stats::Distribution::mean,
                                            ::testing::DoubleEq(1.0))))));
    EXPECT_THAT(server_response_count_view.GetData().distribution_data(),
                ::testing::UnorderedElementsAre(::testing::Pair(
                    ::testing::ElementsAre(server_method_name_),
                    ::testing::AllOf(
                        ::testing::Property(&stats::Distribution::count, i + 1),
                        ::testing::Property(&stats::Distribution::mean,
                                            ::testing::DoubleEq(1.0))))));
  }
}

}  // namespace
}  // namespace testing
}  // namespace opencensus
