// Copyright 2018 OpenCensus Authors
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

#include "opencensus/plugins/grpc/grpc_plugin.h"

#include "absl/time/time.h"
#include "opencensus/stats/internal/aggregation_window.h"
#include "opencensus/stats/internal/set_aggregation_window.h"
#include "opencensus/stats/stats.h"

namespace opencensus {

// These measure definitions should be kept in sync across opencensus
// implementations.

namespace {

stats::Aggregation BytesDistributionAggregation() {
  return stats::Aggregation::Distribution(stats::BucketBoundaries::Explicit(
      {0.0, 1024.0, 2048.0, 4096.0, 16384.0, 65536.0, 262144.0, 1048576.0,
       4194304.0, 16777216.0, 67108864.0, 268435456.0, 1073741824.0,
       4294967296.0}));
}

stats::Aggregation MillisDistributionAggregation() {
  return stats::Aggregation::Distribution(stats::BucketBoundaries::Explicit(
      {0.0,   1.0,    2.0,    3.0,    4.0,     5.0,     6.0,     8.0,     10.0,
       13.0,  16.0,   20.0,   25.0,   30.0,    40.0,    50.0,    65.0,    80.0,
       100.0, 130.0,  160.0,  200.0,  250.0,   300.0,   400.0,   500.0,   650.0,
       800.0, 1000.0, 2000.0, 5000.0, 10000.0, 20000.0, 50000.0, 100000.0}));
}

stats::Aggregation CountDistributionAggregation() {
  return stats::Aggregation::Distribution(
      stats::BucketBoundaries::Exponential(17, 1.0, 2.0));
}

stats::ViewDescriptor MinuteDescriptor() {
  auto descriptor = stats::ViewDescriptor();
  SetAggregationWindow(stats::AggregationWindow::Interval(absl::Minutes(1)),
                       &descriptor);
  return descriptor;
}

stats::ViewDescriptor HourDescriptor() {
  auto descriptor = stats::ViewDescriptor();
  SetAggregationWindow(stats::AggregationWindow::Interval(absl::Hours(1)),
                       &descriptor);
  return descriptor;
}

}  // namespace

void ExperimentalRegisterGrpcViewsForExport() {
  ClientErrorCountCumulative().RegisterForExport();
  ClientRequestBytesCumulative().RegisterForExport();
  ClientResponseBytesCumulative().RegisterForExport();
  ClientRoundtripLatencyCumulative().RegisterForExport();
  ClientServerElapsedTimeCumulative().RegisterForExport();
  ClientStartedCountCumulative().RegisterForExport();
  ClientFinishedCountCumulative().RegisterForExport();
  ClientRequestCountCumulative().RegisterForExport();
  ClientResponseCountCumulative().RegisterForExport();

  ServerErrorCountCumulative().RegisterForExport();
  ServerRequestBytesCumulative().RegisterForExport();
  ServerResponseBytesCumulative().RegisterForExport();
  ServerServerElapsedTimeCumulative().RegisterForExport();
  ServerStartedCountCumulative().RegisterForExport();
  ServerFinishedCountCumulative().RegisterForExport();
  ServerRequestCountCumulative().RegisterForExport();
  ServerResponseCountCumulative().RegisterForExport();
}

// client cumulative
const stats::ViewDescriptor& ClientErrorCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/error_count/cumulative")
          .set_measure(kRpcClientErrorCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey)
          .add_column(kStatusTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRequestBytesCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/request_bytes/cumulative")
          .set_measure(kRpcClientRequestBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientResponseBytesCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/response_bytes/cumulative")
          .set_measure(kRpcClientResponseBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRoundtripLatencyCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/roundtrip_latency/cumulative")
          .set_measure(kRpcClientRoundtripLatencyMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientServerElapsedTimeCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/server_elapsed_time/cumulative")
          .set_measure(kRpcClientServerElapsedTimeMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientStartedCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/started_count/cumulative")
          .set_measure(kRpcClientStartedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientFinishedCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/finished_count/cumulative")
          .set_measure(kRpcClientFinishedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRequestCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/request_count/cumulative")
          .set_measure(kRpcClientRequestCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientResponseCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      stats::ViewDescriptor()
          .set_name("grpc.io/client/response_count/cumulative")
          .set_measure(kRpcClientResponseCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

// server cumulative
const stats::ViewDescriptor& ServerErrorCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/error_count/cumulative")
          .set_measure(kRpcServerErrorCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey)
          .add_column(kStatusTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerRequestBytesCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/request_bytes/cumulative")
          .set_measure(kRpcServerRequestBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerResponseBytesCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/response_bytes/cumulative")
          .set_measure(kRpcServerResponseBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerServerElapsedTimeCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/elapsed_time/cumulative")
          .set_measure(kRpcServerServerElapsedTimeMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerStartedCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/started_count/cumulative")
          .set_measure(kRpcServerStartedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerFinishedCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/finished_count/cumulative")
          .set_measure(kRpcServerFinishedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerRequestCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/request_count/cumulative")
          .set_measure(kRpcServerRequestCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerResponseCountCumulative() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/response_count/cumulative")
          .set_measure(kRpcServerResponseCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

// client minute
const stats::ViewDescriptor& ClientErrorCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/error_count/minute")
          .set_measure(kRpcClientErrorCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey)
          .add_column(kStatusTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRequestBytesMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/request_bytes/minute")
          .set_measure(kRpcClientRequestBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientResponseBytesMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/response_bytes/minute")
          .set_measure(kRpcClientResponseBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRoundtripLatencyMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/roundtrip_latency/minute")
          .set_measure(kRpcClientRoundtripLatencyMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientServerElapsedTimeMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/server_elapsed_time/minute")
          .set_measure(kRpcClientServerElapsedTimeMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientStartedCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/started_count/minute")
          .set_measure(kRpcClientStartedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientFinishedCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/finished_count/minute")
          .set_measure(kRpcClientFinishedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRequestCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/request_count/minute")
          .set_measure(kRpcClientRequestCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientResponseCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/client/response_count/minute")
          .set_measure(kRpcClientResponseCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

// server minute
const stats::ViewDescriptor& ServerErrorCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/error_count/minute")
          .set_measure(kRpcServerErrorCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey)
          .add_column(kStatusTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerRequestBytesMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/request_bytes/minute")
          .set_measure(kRpcServerRequestBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerResponseBytesMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/response_bytes/minute")
          .set_measure(kRpcServerResponseBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerServerElapsedTimeMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/server_elapsed_time/minute")
          .set_measure(kRpcServerServerElapsedTimeMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerStartedCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/started_count/minute")
          .set_measure(kRpcServerStartedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerFinishedCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/finished_count/minute")
          .set_measure(kRpcServerFinishedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerRequestCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/request_count/minute")
          .set_measure(kRpcServerRequestCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerResponseCountMinute() {
  const static stats::ViewDescriptor descriptor =
      MinuteDescriptor()
          .set_name("grpc.io/server/response_count/minute")
          .set_measure(kRpcServerResponseCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

// client hour
const stats::ViewDescriptor& ClientErrorCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/error_count/hour")
          .set_measure(kRpcClientErrorCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey)
          .add_column(kStatusTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRequestBytesHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/request_bytes/hour")
          .set_measure(kRpcClientRequestBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientResponseBytesHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/response_bytes/hour")
          .set_measure(kRpcClientResponseBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRoundtripLatencyHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/roundtrip_latency/hour")
          .set_measure(kRpcClientRoundtripLatencyMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientServerElapsedTimeHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/server_elapsed_time/hour")
          .set_measure(kRpcClientServerElapsedTimeMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientStartedCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/started_count/hour")
          .set_measure(kRpcClientStartedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientFinishedCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/finished_count/hour")
          .set_measure(kRpcClientFinishedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientRequestCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/request_count/hour")
          .set_measure(kRpcClientRequestCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ClientResponseCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/client/response_count/hour")
          .set_measure(kRpcClientResponseCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

// server hour
const stats::ViewDescriptor& ServerErrorCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/error_count/hour")
          .set_measure(kRpcServerErrorCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey)
          .add_column(kStatusTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerRequestBytesHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/request_bytes/hour")
          .set_measure(kRpcServerRequestBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerResponseBytesHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/response_bytes/hour")
          .set_measure(kRpcServerResponseBytesMeasureName)
          .set_aggregation(BytesDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerServerElapsedTimeHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/server_elapsed_time/hour")
          .set_measure(kRpcServerServerElapsedTimeMeasureName)
          .set_aggregation(MillisDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerStartedCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/started_count/hour")
          .set_measure(kRpcServerStartedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerFinishedCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/finished_count/hour")
          .set_measure(kRpcServerFinishedCountMeasureName)
          .set_aggregation(stats::Aggregation::Sum())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerRequestCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/request_count/hour")
          .set_measure(kRpcServerRequestCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

const stats::ViewDescriptor& ServerResponseCountHour() {
  const static stats::ViewDescriptor descriptor =
      HourDescriptor()
          .set_name("grpc.io/server/response_count/hour")
          .set_measure(kRpcServerResponseCountMeasureName)
          .set_aggregation(CountDistributionAggregation())
          .add_column(kMethodTagKey);
  return descriptor;
}

}  // namespace opencensus
