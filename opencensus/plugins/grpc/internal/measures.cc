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

#include "opencensus/plugins/grpc/internal/measures.h"

#include "opencensus/plugins/grpc/grpc_plugin.h"
#include "opencensus/stats/stats.h"

namespace opencensus {

// These measure definitions should be kept in sync across opencensus
// implementations--see
// https://github.com/census-instrumentation/opencensus-java/blob/master/contrib/grpc_metrics/src/main/java/io/opencensus/contrib/grpc/metrics/RpcMeasureConstants.java.

namespace {

// Unit constants
constexpr char kUnitBytes[] = "By";
constexpr char kUnitMilliseconds[] = "ms";
constexpr char kCount[] = "1";

}  // namespace

// Client
stats::MeasureInt64 RpcClientErrorCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcClientErrorCountMeasureName, "RPC Errors", kCount);
  return measure;
}

stats::MeasureDouble RpcClientRequestBytes() {
  static stats::MeasureDouble measure = stats::MeasureDouble::Register(
      kRpcClientRequestBytesMeasureName, "Request bytes", kUnitBytes);
  return measure;
}

stats::MeasureDouble RpcClientResponseBytes() {
  static stats::MeasureDouble measure = stats::MeasureDouble::Register(
      kRpcClientResponseBytesMeasureName, "Response bytes", kUnitBytes);
  return measure;
}

stats::MeasureDouble RpcClientRoundtripLatency() {
  static stats::MeasureDouble measure = stats::MeasureDouble::Register(
      kRpcClientRoundtripLatencyMeasureName, "RPC roundtrip latency msec",
      kUnitMilliseconds);
  return measure;
}

stats::MeasureDouble RpcClientServerElapsedTime() {
  static stats::MeasureDouble measure = stats::MeasureDouble::Register(
      kRpcClientServerElapsedTimeMeasureName, "Server elapsed time in msecs",
      kUnitMilliseconds);
  return measure;
}

stats::MeasureInt64 RpcClientStartedCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcClientStartedCountMeasureName,
      "Number of client RPCs (streams) started", kCount);
  return measure;
}

stats::MeasureInt64 RpcClientFinishedCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcClientFinishedCountMeasureName,
      "Number of client RPCs (streams) finished", kCount);
  return measure;
}

stats::MeasureInt64 RpcClientRequestCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcClientRequestCountMeasureName,
      "Number of client RPC request messages", kCount);
  return measure;
}

stats::MeasureInt64 RpcClientResponseCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcClientResponseCountMeasureName,
      "Number of client RPC response messages", kCount);
  return measure;
}

// Server
stats::MeasureInt64 RpcServerErrorCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcServerErrorCountMeasureName, "RPC Errors", kCount);
  return measure;
}

stats::MeasureDouble RpcServerRequestBytes() {
  static stats::MeasureDouble measure = stats::MeasureDouble::Register(
      kRpcServerRequestBytesMeasureName, "Request bytes", kUnitBytes);
  return measure;
}

stats::MeasureDouble RpcServerResponseBytes() {
  static stats::MeasureDouble measure = stats::MeasureDouble::Register(
      kRpcServerResponseBytesMeasureName, "Response bytes", kUnitBytes);
  return measure;
}

stats::MeasureDouble RpcServerServerElapsedTime() {
  static stats::MeasureDouble measure = stats::MeasureDouble::Register(
      kRpcServerServerElapsedTimeMeasureName, "Server elapsed time in msecs",
      kUnitMilliseconds);
  return measure;
}

stats::MeasureInt64 RpcServerStartedCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcServerStartedCountMeasureName,
      "Number of server RPCs (streams) started", kCount);
  return measure;
}

stats::MeasureInt64 RpcServerFinishedCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcServerFinishedCountMeasureName,
      "Number of server RPCs (streams) finished", kCount);
  return measure;
}

stats::MeasureInt64 RpcServerRequestCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcServerRequestCountMeasureName,
      "Number of server RPC request messages", kCount);
  return measure;
}

stats::MeasureInt64 RpcServerResponseCount() {
  static stats::MeasureInt64 measure = stats::MeasureInt64::Register(
      kRpcServerResponseCountMeasureName,
      "Number of server RPC response messages", kCount);
  return measure;
}

}  // namespace opencensus
