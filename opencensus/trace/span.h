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

#ifndef OPENCENSUS_TRACE_SPAN_H_
#define OPENCENSUS_TRACE_SPAN_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "opencensus/trace/attribute_value_ref.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span_context.h"
#include "opencensus/trace/status_code.h"
#include "opencensus/trace/trace_config.h"
#include "opencensus/trace/trace_params.h"

namespace opencensus {
class CensusContext;
namespace trace {

namespace exporter {
class LocalSpanStoreImpl;
class RunningSpanStoreImpl;
}  // namespace exporter

class Span;
class SpanGenerator;
class SpanImpl;
class SpanTestPeer;

// AttributesRef is an initializer list of key-value pairs, used to pass
// Attributes to the tracing API. e.g.:
//   AddAttributes({{"key1", "value1"}, {"key2", 123}});
using AttributesRef =
    absl::Span<const std::pair<absl::string_view, AttributeValueRef>>;

// Options for Starting a Span.
struct StartSpanOptions {
  StartSpanOptions(
      Sampler* sampler = nullptr,  // Default Sampler.
      bool record_events = false,  // Only record events if the Span is sampled.
      const std::vector<Span*>& parent_links = {})
      : sampler(sampler),
        record_events(record_events),
        parent_links(parent_links) {}

  // The Sampler to use. It must remain valid for the duration of the
  // StartSpan() call. If nullptr, use the default Sampler from TraceConfig.
  //
  // A Span that's sampled will be exported (see exporter/span_exporter.h).
  // All sampled Spans record events.
  const Sampler* sampler;

  // This option can be used to request recording of events for non-sampled
  // Spans. Spans that record events show up in the RunningSpanStore and
  // LocalSpanStore in the running process.
  const bool record_events;

  // Pointers to Spans in *other Traces* that are parents of this Span. They
  // must remain valid for the duration of the StartSpan() call.
  const std::vector<Span*> parent_links;
};

// Span represents a trace span. It has a SpanContext. Span is thread-safe.
class Span final {
 public:
  // Constructs a no-op Span with an invalid context. Attempts to add
  // attributes, etc, will all be no-ops.
  static Span BlankSpan();

  // Constructs a root Span (if parent is nullptr) or a Span with a local
  // parent.
  //
  // Example for root span:
  //   auto root_span = ::opencensus::trace::Span::StartSpan("MyOperation");
  //
  // Example for child span:
  //   // Constructing a ProbabilitySampler can be expensive.
  //   static ::opencensus::trace::ProbabilitySampler sampler(0.1);
  //   auto child_span = ::opencensus::trace::Span::StartSpan(
  //       "SubOperation", &root_span, {&sampler});
  static Span StartSpan(absl::string_view name, const Span* parent = nullptr,
                        const StartSpanOptions& options = StartSpanOptions());

  // Constructs a span with a remote parent.
  static Span StartSpanWithRemoteParent(
      absl::string_view name, const SpanContext& parent_ctx,
      const StartSpanOptions& options = StartSpanOptions());

  // Attempts to insert an attribute into the Span, unless it already exists in
  // which case it will update the value of that attribute. If the max number of
  // attributes is exceeded, one of the previous attributes will be evicted.
  // AddAttributes is faster due to batching.
  void AddAttribute(absl::string_view key, AttributeValueRef attribute);
  void AddAttributes(AttributesRef attributes);

  // Adds an Annotation to the Span. If the max number of Annotations is
  // exceeded, an Annotation will be evicted in a FIFO manner.
  // In future, there will be a limit of 4 attributes per annotation.
  void AddAnnotation(absl::string_view description,
                     AttributesRef attributes = {});

  // Adds a MessageEvent to the Span. If the max number of MessageEvents is
  // exceeded, a MessageEvent will be evicted in a FIFO manner.
  void AddSentMessageEvent(uint32_t message_id,
                           uint32_t compressed_message_size,
                           uint32_t uncompressed_message_size);
  void AddReceivedMessageEvent(uint32_t message_id,
                               uint32_t compressed_message_size,
                               uint32_t uncompressed_message_size);

  // Adds a Link to the Span. If the max number of Links is exceeded, a Link
  // will be evicted in a FIFO manner. In future, there will be a limit of 32
  // attributes per link.
  void AddParentLink(const SpanContext& parent_ctx,
                     AttributesRef attributes = {});
  void AddChildLink(const SpanContext& child_ctx,
                    AttributesRef attributes = {});

  // Sets the status of the Span. See status_code.h for canonical codes.
  void SetStatus(StatusCode canonical_code, absl::string_view message = "");

  // Marks the end of a Span. No further changes can be made to the Span after
  // End is called.
  void End();

  // Returns the SpanContext associated with this Span.
  const SpanContext& context() const;

  // Returns true if the Span is sampled (will be exported).
  // Sampled spans always record events.
  bool IsSampled() const;

  // Returns true if the Span is recording events (will appear in Span stores).
  // Sampled spans always record events, but not all Spans that are recording
  // are sampled.
  bool IsRecording() const;

 private:
  Span() {}
  Span(const SpanContext& context, SpanImpl* impl);

  // Returns span_impl_, only used for testing.
  std::shared_ptr<SpanImpl> span_impl_for_test() { return span_impl_; }

  // Spans that aren't sampled still have a valid SpanContext that propagates,
  // but no span_impl_.
  const SpanContext context_;

  // Shared pointer to the underlying Span representation. This is nullptr for
  // Spans which are not recording events.
  std::shared_ptr<SpanImpl> span_impl_;

  friend class ::opencensus::trace::exporter::RunningSpanStoreImpl;
  friend class ::opencensus::trace::exporter::LocalSpanStoreImpl;
  friend class ::opencensus::trace::SpanTestPeer;
  friend class ::opencensus::trace::SpanGenerator;
  friend class ::opencensus::CensusContext;
};

}  // namespace trace
}  // namespace opencensus

#endif  // OPENCENSUS_TRACE_SPAN_H_
