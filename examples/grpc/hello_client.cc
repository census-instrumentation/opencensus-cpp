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

#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "examples/grpc/exporters.h"
#include "examples/grpc/hello.grpc.pb.h"
#include "examples/grpc/hello.pb.h"
#include "opencensus/tags/tag_key.h"
#include "opencensus/tags/tag_map.h"
#include "opencensus/tags/with_tag_map.h"
#include "opencensus/trace/context_util.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/with_span.h"

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

  RegisterExporters();

  // Create a Channel to send RPCs over.
  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(hostport, grpc::InsecureChannelCredentials());
  std::unique_ptr<HelloService::Stub> stub = HelloService::NewStub(channel);

  // Create a span, this will be the parent of the RPC span.
  static opencensus::trace::AlwaysSampler sampler;
  auto span = opencensus::trace::Span::StartSpan(
      "HelloClient", /*parent=*/nullptr, {&sampler});
  std::cout << "HelloClient span context is " << span.context().ToString()
            << "\n";

  // Create a tag map.
  static const auto key = opencensus::tags::TagKey::Register("my_key");
  opencensus::tags::TagMap tags({{key, "my_value"}});

  {
    opencensus::trace::WithSpan ws(span);
    opencensus::tags::WithTagMap wt(tags);

    // The client Span ends when ctx falls out of scope.
    grpc::ClientContext ctx;
    ctx.AddMetadata("key1", "value1");

    HelloRequest request;
    HelloReply reply;
    request.set_name(name);
    std::cout << "Sending request: \"" << request.ShortDebugString() << "\"\n";
    opencensus::trace::GetCurrentSpan().AddAnnotation("Sending request.");

    // Send the RPC.
    grpc::Status status = stub->SayHello(&ctx, request, &reply);

    std::cout << "Got status: " << status.error_code() << ": \""
              << status.error_message() << "\"\n";
    std::cout << "Got reply: \"" << reply.ShortDebugString() << "\"\n";
    opencensus::trace::GetCurrentSpan().AddAnnotation(
        "Got reply.", {{"status", absl::StrCat(status.error_code(), ": ",
                                               status.error_message())}});
    if (!status.ok()) {
      // TODO: Map grpc::StatusCode to trace::StatusCode.
      opencensus::trace::GetCurrentSpan().SetStatus(
          opencensus::trace::StatusCode::UNKNOWN, status.error_message());
    }
    // Don't forget to explicitly end the span!
    opencensus::trace::GetCurrentSpan().End();
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
