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

#ifndef OPENCENSUS_EXPORTERS_TRACE_STACKDRIVER_STACKDRIVER_EXPORTER_H_
#define OPENCENSUS_EXPORTERS_TRACE_STACKDRIVER_STACKDRIVER_EXPORTER_H_

#include <memory>
#include <string>
#include <vector>

#include "google/devtools/cloudtrace/v2/tracing.grpc.pb.h"
#include "include/grpc++/grpc++.h"
#include "opencensus/trace/exporter/span_data.h"
#include "opencensus/trace/exporter/span_exporter.h"

namespace opencensus {
namespace exporters {
namespace trace {

class StackdriverExporter
    : public ::opencensus::trace::exporter::SpanExporter::Handler {
 public:
  StackdriverExporter(absl::string_view project_id) : project_id_(project_id) {}
  void Export(const std::vector<::opencensus::trace::exporter::SpanData>& spans)
      override;

  // Registers the exporter and sets the project Id. The following are required
  // to communicate with stackdriver: a valid project Id, a valid authentication
  // key which has been generated for that project, and the server security
  // credentials. An authentication key can be generated for your project in the
  // gcp/stackdriver settings for your account. Grpc will look for the key using
  // the path defined by GOOGLE_APPLICATION_CREDENTIALS environment variable
  // (export GOOGLE_APPLICATION_CREDENTIALS=<path_to_key>). The server security
  // credentials located in <grpc_path>/etc/roots.pem needs to be copied to
  // /usr/share/grpc/roots.pem
  static void Register(absl::string_view project_id);

 private:
  class TraceClient {
   public:
    TraceClient(const std::shared_ptr<grpc::Channel>& channel)
        : stub_(google::devtools::cloudtrace::v2::TraceService::NewStub(
              channel)) {}

    // Packages a batch of spans into a single request and writes it to
    // stackdriver. Returns the status of the operation.
    grpc::Status BatchWriteSpans(
        const ::google::devtools::cloudtrace::v2::BatchWriteSpansRequest&
            request);

   private:
    std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::Stub> stub_;
  };

  friend class StackdriverExporterTestPeer;

  static void ExportForTesting(
      absl::string_view project_id,
      const std::vector<::opencensus::trace::exporter::SpanData>& spans);

  const std::string project_id_;
  std::unique_ptr<TraceClient> trace_client_;
};

}  // namespace trace
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_TRACE_STACKDRIVER_STACKDRIVER_EXPORTER_H_
