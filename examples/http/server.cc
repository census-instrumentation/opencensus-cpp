// Copyright 2019, OpenCensus Authors
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

// Example HTTP server with OpenCensus instrumentation.

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "CivetServer.h"
#include "absl/strings/escaping.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/stats/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/trace/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/zipkin/zipkin_exporter.h"
#include "opencensus/trace/propagation/trace_context.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"
#include "opencensus/trace/trace_config.h"

namespace {

using opencensus::exporters::trace::ZipkinExporter;
using opencensus::exporters::trace::ZipkinExporterOptions;

class CounterHandler : public CivetHandler {
 public:
  bool handleGet(CivetServer *server, struct mg_connection *conn) {
    // Handle incoming trace context.
    ::opencensus::trace::SpanContext parent;
    const char *traceparent = server->getHeader(conn, "traceparent");
    if (traceparent != nullptr) {
      parent =
          opencensus::trace::propagation::FromTraceParentHeader(traceparent);
    }
    std::cerr << "parent is " << parent.ToString() << "\n";
    auto span = parent.IsValid()
                    ? ::opencensus::trace::Span::StartSpanWithRemoteParent(
                          "Counter", parent)
                    : ::opencensus::trace::Span::StartSpan("Counter");

    const mg_request_info *req = mg_get_request_info(conn);
    printf("Got %d headers:\n", req->num_headers);
    for (int i = 0; i < req->num_headers; ++i) {
      printf("  %s: %s\n", req->http_headers[i].name,
             req->http_headers[i].value);
    }

    // Perform work.
    counter_++;
    span.AddAnnotation("Incremented counter.", {{"counter_value", counter_}});
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");
    mg_printf(conn, "<!DOCTYPE html>\n");
    mg_printf(conn, "<html><body>There have been %d hits.</body></html>\n",
              counter_);
    span.AddAnnotation("Built response.");
    span.End();
    return true;
  }

 private:
  int counter_ = 0;
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

  // For debugging, register exporters that just write to stdout.
  opencensus::exporters::stats::StdoutExporter::Register();
  opencensus::exporters::trace::StdoutExporter::Register();

  ZipkinExporterOptions options("http://127.0.0.1:9411/api/v2/spans");
  options.service_name = "counter_server";
  ZipkinExporter::Register(options);

  CivetServer server{
      {"listening_ports", absl::StrCat(port), "num_threads", "2"}};
  CounterHandler handler;
  server.addHandler("/", handler);

  std::cout << "Server listening on port " << port << "\n";
  for (;;) {
    absl::SleepFor(absl::Seconds(1));
  }
}
