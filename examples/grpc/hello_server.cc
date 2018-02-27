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

// Example RPC server using gRPC and OpenCensus and Stackdriver.

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "examples/hello.grpc.pb.h"
#include "examples/hello.pb.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/stats/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/trace/stdout/stdout_exporter.h"
#include "opencensus/plugins/grpc/grpc_plugin.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/sampler.h"
#include "prometheus/exposer.h"

namespace {

using examples::HelloReply;
using examples::HelloRequest;
using examples::HelloService;

class HelloServiceImpl final : public HelloService::Service {
  grpc::Status SayHello(grpc::ServerContext* context,
                        const HelloRequest* request,
                        HelloReply* reply) override {
    opencensus::trace::Span span =
        opencensus::GetSpanFromServerContext(context);
    span.AddAnnotation("Sleeping.");
    absl::SleepFor(absl::Milliseconds(50));
    span.AddAnnotation("Constructing greeting.", {{"name", request->name()}});
    reply->set_message(absl::StrCat("Hello ", request->name(), "!"));
    // TODO: Record() custom stats.
    std::cerr << "SayHello RPC handled.\n";
    return grpc::Status::OK;
  }
};

}  // namespace

int main(int argc, char** argv) {
  // Handle port argument.
  int port = 0;
  if (argc == 2) {
    if (!absl::SimpleAtoi(argv[1], &port)) {
      std::cerr << "Invalid port number: \"" << argv[1] << "\"";
      return 1;
    }
  }

  // Register the OpenCensus gRPC plugin to enable stats and tracing in gRPC.
  opencensus::RegisterGrpcPlugin();

  // Trace all RPCs.
  // FIXME: this causes an endless stream of stackdriver write RPCs to be sampled :^)
  opencensus::trace::TraceConfig::SetCurrentTraceParams({128, 128, 128, 128, opencensus::trace::ProbabilitySampler(1.0)});

  // Register exporters for Stackdriver.
  const char* project_id = getenv("STACKDRIVER_PROJECT_ID");
  if (project_id == nullptr) {
    std::cerr
        << "The STACKDRIVER_PROJECT_ID environment variable is not set: not exporting to Stackdriver.\n";
  } else {
    opencensus::exporters::stats::StackdriverExporter::Register(project_id,
      absl::StrCat("hello_server", 123));
    opencensus::exporters::trace::StackdriverExporter::Register(project_id);
  }

  // For debugging, register exporters that just write to stdout.
  opencensus::exporters::stats::StdoutExporter::Register();
  opencensus::exporters::trace::StdoutExporter::Register();

  // Keep a shared pointer to the Prometheus exporter.
  auto exporter =
      std::make_shared<opencensus::exporters::stats::PrometheusExporter>();

  // Expose a Prometheus endpoint.
  prometheus::Exposer exposer("127.0.0.1:8080");
  exposer.RegisterCollectable(exporter);

  // Add views for RPC stats.
  const auto server_request_bytes_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_measure(opencensus::kRpcServerRequestBytesMeasureName)
          .set_name("server_request_bytes")
          .set_aggregation(opencensus::stats::Aggregation::Distribution(
              opencensus::stats::BucketBoundaries::Exponential(5, 1, 10)))
          .add_column(opencensus::kMethodTagKey);
  const auto server_response_bytes_descriptor =
      opencensus::stats::ViewDescriptor()
          .set_measure(opencensus::kRpcServerResponseBytesMeasureName)
          .set_name("server_response_bytes")
          .set_aggregation(opencensus::stats::Aggregation::Distribution(
              opencensus::stats::BucketBoundaries::Explicit({})))
          .add_column(opencensus::kMethodTagKey);

  // TODO: Get default/example gRPC views from plugin.

  opencensus::stats::StatsExporter::AddView(server_request_bytes_descriptor);
  opencensus::stats::StatsExporter::AddView(server_response_bytes_descriptor);

  // Start the RPC server. You shouldn't see any output from gRPC before this.
  std::cerr << "gRPC starting.\n";
  HelloServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(absl::StrCat("[::]:", port),
                           grpc::InsecureServerCredentials(), &port);
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on [::]:" << port << "\n";
  server->Wait();
}
