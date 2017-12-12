// Copyright 2017, OpenCensus Authors
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

#ifndef OPENCENSUS_TRACE_EXPORTER_SPAN_EXPORTER_H_
#define OPENCENSUS_TRACE_EXPORTER_SPAN_EXPORTER_H_

#include <memory>
#include <vector>

#include "opencensus/trace/exporter/span_data.h"

namespace opencensus {
namespace trace {
namespace exporter {

class SpanExporterImpl;

// SpanExporter tracks Exporters. Thread-safe.
class SpanExporter final {
 public:
  // Handlers allow different tracing services to export recorded data for
  // sampled spans in their own format.
  //
  // To export data, the Handler MUST be registered to the global SpanExporter
  // using RegisterHandler().
  class Handler {
   public:
    virtual ~Handler() = default;

   private:
    friend class SpanExporterImpl;

    // Exports sampled (see TraceOptions::IsSampled()) Spans using the immutable
    // SpanData representation.
    //
    // The implementation SHOULD NOT block the calling thread. It should execute
    // the export on a different thread if possible. This function must be
    // thread-safe.
    virtual void Export(const std::vector<SpanData>& spans) = 0;
  };

  // Register a handler that's used to export SpanData for sampled Spans.
  static void Register(std::unique_ptr<Handler> handler);
};

}  // namespace exporter
}  // namespace trace
}  // namespace opencensus

#endif  // OPENCENSUS_TRACE_EXPORTER_SPAN_EXPORTER_H_
