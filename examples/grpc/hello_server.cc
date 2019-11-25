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

// Example RPC server using gRPC.

#include <grpcpp/grpcpp.h>
#include <grpcpp/opencensus.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "absl/strings/escaping.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "examples/grpc/exporters.h"
#include "examples/grpc/hello.grpc.pb.h"
#include "examples/grpc/hello.pb.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"
#include "opencensus/stats/stats.h"
#include "opencensus/tags/context_util.h"
#include "opencensus/tags/tag_map.h"
#include "opencensus/trace/context_util.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"
#include "opencensus/trace/trace_config.h"
#include "prometheus/exposer.h"

namespace {

using examples::HelloReply;
using examples::HelloRequest;
using examples::HelloService;

ABSL_CONST_INIT const char kLettersMeasureName[] =
    "example.org/measure/letters";

opencensus::stats::MeasureInt64 LettersMeasure() {
  static const opencensus::stats::MeasureInt64 measure =
      opencensus::stats::MeasureInt64::Register(
          kLettersMeasureName, "Number of letters in processed names.", "By");
  return measure;
}

opencensus::tags::TagKey CaseKey() {
  static const opencensus::tags::TagKey key =
      opencensus::tags::TagKey::Register("example_uppercased");
  return key;
}

absl::string_view ToStringView(const ::grpc::string_ref &s) {
  return absl::string_view(s.data(), s.size());
}

// A helper function that performs some work in its own Span.
void PerformWork(opencensus::trace::Span *parent) {
  auto span = opencensus::trace::Span::StartSpan("internal_work", parent);
  span.AddAttribute("my_attribute", "blue");
  span.AddAnnotation("Performing work.");
  absl::SleepFor(absl::Milliseconds(20));  // Working hard here.
  span.End();
}

class HelloServiceImpl final : public HelloService::Service {
  grpc::Status SayHello(grpc::ServerContext *context,
                        const HelloRequest *request,
                        HelloReply *reply) override {
    opencensus::trace::Span span = grpc::GetSpanFromServerContext(context);
    span.AddAttribute("my_attribute", "red");
    span.AddAnnotation(
        "Constructing greeting.",
        {{"name", request->name()}, {"name length", request->name().size()}});
    reply->set_message(absl::StrCat("Hello ", request->name(), "!"));
    absl::SleepFor(absl::Milliseconds(10));
    PerformWork(&span);
    span.AddAnnotation("Sleeping.");
    absl::SleepFor(absl::Milliseconds(30));
    // Record custom stats.
    opencensus::stats::Record(
        {{LettersMeasure(), request->name().size()}},
        {{CaseKey(), isupper(request->name()[0]) ? "upper" : "lower"}});
    // Give feedback on stderr.
    std::cerr << "SayHello RPC handler:\n";
    std::cerr << "  Current context: "
              << opencensus::trace::GetCurrentSpan().context().ToString()
              << "\n";
    std::cerr << "  Current tags: "
              << opencensus::tags::GetCurrentTagMap().DebugString() << "\n";
    std::cerr << "  gRPC metadata:\n";
    auto metadata = context->client_metadata();
    for (const auto &mdpair : metadata) {
      std::cerr << "    \"" << absl::CEscape(ToStringView(mdpair.first))
                << "\": \"" << absl::CEscape(ToStringView(mdpair.second))
                << "\"\n";
    }
    std::cerr << "  (end of metadata)\n";
    return grpc::Status::OK;
  }
};

}  // namespace

int main(int argc, char **argv) {
  // Handle port argument.
  int port = 0;
  if (argc == 2) {
    if (!absl::SimpleAtoi(argv[1], &port)) {
      std::cerr << "Invalid port number: \"" << argv[1] << "\"";
      return 1;
    }
  }

  // Register the OpenCensus gRPC plugin to enable stats and tracing in gRPC.
  grpc::RegisterOpenCensusPlugin();

  // Register the gRPC views (latency, error count, etc).
  grpc::RegisterOpenCensusViewsForExport();

  RegisterExporters();

  // Keep a shared pointer to the Prometheus exporter.
  auto exporter =
      std::make_shared<opencensus::exporters::stats::PrometheusExporter>();

  // Expose a Prometheus endpoint.
  prometheus::Exposer exposer("127.0.0.1:8080");
  exposer.RegisterCollectable(exporter);

  // Init custom measure.
  LettersMeasure();

  // Add a View for custom stats.
  const opencensus::stats::ViewDescriptor letters_view =
      opencensus::stats::ViewDescriptor()
          .set_name("example.org/view/letters_view")
          .set_description("number of letters in names greeted over time")
          .set_measure(kLettersMeasureName)
          .set_aggregation(opencensus::stats::Aggregation::Sum())
          .add_column(CaseKey());
  opencensus::stats::View view(letters_view);
  assert(view.IsValid());
  letters_view.RegisterForExport();

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
