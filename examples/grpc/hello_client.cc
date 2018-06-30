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
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "examples/grpc/hello.grpc.pb.h"
#include "examples/grpc/hello.pb.h"
#include "examples/grpc/stackdriver.h"
#include "include/grpcpp/opencensus.h"
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/stats/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/trace/stdout/stdout_exporter.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/trace_config.h"

namespace {

using examples::HelloReply;
using examples::HelloRequest;
using examples::HelloService;

}  // namespace

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " host:port [name]\n";
    return 1;
  }
  const std::string hostport = argv[1];
  std::string name = "world";
  if (argc >= 3) {
    name = argv[2];
  }

  // Register the OpenCensus gRPC plugin to enable stats and tracing in gRPC.
  grpc::RegisterOpenCensusPlugin();

  // Register exporters for Stackdriver.
  RegisterStackdriverExporters();

  // Trace all outgoing RPCs.
  opencensus::trace::TraceConfig::SetCurrentTraceParams(
      {128, 128, 128, 128, opencensus::trace::ProbabilitySampler(1.0)});

  // For debugging, register exporters that just write to stdout.
  opencensus::exporters::stats::StdoutExporter::Register();
  opencensus::exporters::trace::StdoutExporter::Register();

  // Create a Channel to send RPCs over.
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(hostport, grpc::InsecureChannelCredentials());
  std::unique_ptr<HelloService::Stub> stub = HelloService::NewStub(channel);

  // Send the RPC.
  {
    // The client Span ends when ctx falls out of scope.
    grpc::ClientContext ctx;
    HelloRequest request;
    HelloReply reply;
    request.set_name(name);
    std::cout << "Sending request: \"" << request.ShortDebugString() << "\"\n";

    grpc::Status status = stub->SayHello(&ctx, request, &reply);

    std::cout << "Got status: " << status.error_code() << ": \""
              << status.error_message() << "\"\n";
    std::cout << "Got reply: \"" << reply.ShortDebugString() << "\"\n";
  }

  // Disable tracing any further RPCs (which will be sent by exporters).
  opencensus::trace::TraceConfig::SetCurrentTraceParams(
      {128, 128, 128, 128, opencensus::trace::ProbabilitySampler(0.0)});

  // Sleep while exporters run in the background.
  std::cout << "Client sleeping, ^C to exit.\n";
  while (true) {
    absl::SleepFor(absl::Seconds(10));
  }
}
