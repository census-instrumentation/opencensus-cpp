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

#ifndef OPENCENSUS_PLUGINS_GRPC_GRPC_PLUGIN_H_
#define OPENCENSUS_PLUGINS_GRPC_GRPC_PLUGIN_H_

#include "absl/strings/string_view.h"
#include "opencensus/stats/stats.h"
#include "opencensus/trace/span.h"

namespace grpc {
class ServerContext;
}

namespace opencensus {

// Registers the OpenCensus plugin with gRPC, so that it will be used for future
// RPCs. This must be called before any views are created on the measures
// defined below.
void RegisterGrpcPlugin();

// Registers the cumulative gRPC views so that they will be exported by any
// registered stats exporter.
// For on-task stats, construct a View using the ViewDescriptors below.
// Experimental: These view definitions are subject to change.
void ExperimentalRegisterGrpcViewsForExport();

// Returns the tracing Span for the current RPC.
opencensus::trace::Span GetSpanFromServerContext(grpc::ServerContext* context);

// The tag key for the RPC method and status, set for all values recorded for
// the following measures.
extern const absl::string_view kMethodTagKey;
extern const absl::string_view kStatusTagKey;

// Names of measures used by the plugin--users can create views on these
// measures but should not record data for them.
extern const absl::string_view kRpcClientErrorCountMeasureName;
extern const absl::string_view kRpcClientRequestBytesMeasureName;
extern const absl::string_view kRpcClientResponseBytesMeasureName;
extern const absl::string_view kRpcClientRoundtripLatencyMeasureName;
extern const absl::string_view kRpcClientServerElapsedTimeMeasureName;
extern const absl::string_view kRpcClientStartedCountMeasureName;
extern const absl::string_view kRpcClientFinishedCountMeasureName;
extern const absl::string_view kRpcClientRequestCountMeasureName;
extern const absl::string_view kRpcClientResponseCountMeasureName;

extern const absl::string_view kRpcServerErrorCountMeasureName;
extern const absl::string_view kRpcServerRequestBytesMeasureName;
extern const absl::string_view kRpcServerResponseBytesMeasureName;
extern const absl::string_view kRpcServerServerElapsedTimeMeasureName;
extern const absl::string_view kRpcServerStartedCountMeasureName;
extern const absl::string_view kRpcServerFinishedCountMeasureName;
extern const absl::string_view kRpcServerRequestCountMeasureName;
extern const absl::string_view kRpcServerResponseCountMeasureName;

// Canonical gRPC view definitions.
// These view definitions are subject to change.
const stats::ViewDescriptor& ClientErrorCountCumulative();
const stats::ViewDescriptor& ClientRequestBytesCumulative();
const stats::ViewDescriptor& ClientResponseBytesCumulative();
const stats::ViewDescriptor& ClientRoundtripLatencyCumulative();
const stats::ViewDescriptor& ClientServerElapsedTimeCumulative();
const stats::ViewDescriptor& ClientStartedCountCumulative();
const stats::ViewDescriptor& ClientFinishedCountCumulative();
const stats::ViewDescriptor& ClientRequestCountCumulative();
const stats::ViewDescriptor& ClientResponseCountCumulative();

const stats::ViewDescriptor& ServerErrorCountCumulative();
const stats::ViewDescriptor& ServerRequestBytesCumulative();
const stats::ViewDescriptor& ServerResponseBytesCumulative();
const stats::ViewDescriptor& ServerServerElapsedTimeCumulative();
const stats::ViewDescriptor& ServerStartedCountCumulative();
const stats::ViewDescriptor& ServerFinishedCountCumulative();
const stats::ViewDescriptor& ServerRequestCountCumulative();
const stats::ViewDescriptor& ServerResponseCountCumulative();

const stats::ViewDescriptor& ClientErrorCountMinute();
const stats::ViewDescriptor& ClientRequestBytesMinute();
const stats::ViewDescriptor& ClientResponseBytesMinute();
const stats::ViewDescriptor& ClientRoundtripLatencyMinute();
const stats::ViewDescriptor& ClientServerElapsedTimeMinute();
const stats::ViewDescriptor& ClientStartedCountMinute();
const stats::ViewDescriptor& ClientFinishedCountMinute();
const stats::ViewDescriptor& ClientRequestCountMinute();
const stats::ViewDescriptor& ClientResponseCountMinute();

const stats::ViewDescriptor& ServerErrorCountMinute();
const stats::ViewDescriptor& ServerRequestBytesMinute();
const stats::ViewDescriptor& ServerResponseBytesMinute();
const stats::ViewDescriptor& ServerServerElapsedTimeMinute();
const stats::ViewDescriptor& ServerStartedCountMinute();
const stats::ViewDescriptor& ServerFinishedCountMinute();
const stats::ViewDescriptor& ServerRequestCountMinute();
const stats::ViewDescriptor& ServerResponseCountMinute();

const stats::ViewDescriptor& ClientErrorCountHour();
const stats::ViewDescriptor& ClientRequestBytesHour();
const stats::ViewDescriptor& ClientResponseBytesHour();
const stats::ViewDescriptor& ClientRoundtripLatencyHour();
const stats::ViewDescriptor& ClientServerElapsedTimeHour();
const stats::ViewDescriptor& ClientStartedCountHour();
const stats::ViewDescriptor& ClientFinishedCountHour();
const stats::ViewDescriptor& ClientRequestCountHour();
const stats::ViewDescriptor& ClientResponseCountHour();

const stats::ViewDescriptor& ServerErrorCountHour();
const stats::ViewDescriptor& ServerRequestBytesHour();
const stats::ViewDescriptor& ServerResponseBytesHour();
const stats::ViewDescriptor& ServerServerElapsedTimeHour();
const stats::ViewDescriptor& ServerStartedCountHour();
const stats::ViewDescriptor& ServerFinishedCountHour();
const stats::ViewDescriptor& ServerRequestCountHour();
const stats::ViewDescriptor& ServerResponseCountHour();

}  // namespace opencensus

#endif  // OPENCENSUS_PLUGINS_GRPC_GRPC_PLUGIN_H_
