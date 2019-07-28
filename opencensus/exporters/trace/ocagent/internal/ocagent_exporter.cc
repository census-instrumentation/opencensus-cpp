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

#include "opencensus/exporters/trace/ocagent/ocagent_exporter.h"

#include <cstdint>
#include <iostream>
#include <unistd.h>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "opencensus/common/internal/grpc/status.h"
#include "opencensus/common/internal/grpc/with_user_agent.h"
#include "opencensus/common/internal/hostname.h"
#include "opencensus/common/version.h"
#include "opencensus/trace/exporter/span_data.h"
#include "opencensus/trace/exporter/span_exporter.h"

#include <grpcpp/grpcpp.h>

namespace opencensus {
namespace exporters {
namespace trace {
namespace {

constexpr size_t kAttributeStringLen = 256;
constexpr size_t kAnnotationStringLen = 256;
constexpr size_t kDisplayNameStringLen = 128;

bool Validate(const google::protobuf::Timestamp &t) {
  const auto sec = t.seconds();
  const auto ns = t.nanos();
  // sec must be [0001-01-01T00:00:00Z, 9999-12-31T23:59:59.999999999Z]
  if (sec < -62135596800 || sec > 253402300799) {
    return false;
  }
  if (ns < 0 || ns > 999999999) {
    return false;
  }
  return true;
}

bool EncodeTimestampProto(absl::Time t, google::protobuf::Timestamp *proto) {
  const int64_t s = absl::ToUnixSeconds(t);
  proto->set_seconds(s);
  proto->set_nanos((t - absl::FromUnixSeconds(s)) / absl::Nanoseconds(1));
  return Validate(*proto);
}

void SetTruncatableString(
    absl::string_view str, size_t max_len,
    ::opencensus::proto::trace::v1::TruncatableString *t_str) {
  if (str.size() > max_len) {
    t_str->set_value(std::string(str.substr(0, max_len)));
    t_str->set_truncated_byte_count(str.size() - max_len);
  } else {
    t_str->set_value(std::string(str));
    t_str->set_truncated_byte_count(0);
  }
}

::opencensus::proto::trace::v1::Span_Link_Type
ConvertLinkType(::opencensus::trace::exporter::Link::Type type) {
  switch (type) {
  case ::opencensus::trace::exporter::Link::Type::kChildLinkedSpan:
    return ::opencensus::proto::trace::v1::Span_Link_Type_CHILD_LINKED_SPAN;
  case ::opencensus::trace::exporter::Link::Type::kParentLinkedSpan:
    return ::opencensus::proto::trace::v1::Span_Link_Type_PARENT_LINKED_SPAN;
  }
  return ::opencensus::proto::trace::v1::Span_Link_Type_TYPE_UNSPECIFIED;
}

::opencensus::proto::trace::v1::Span_TimeEvent_MessageEvent_Type
ConvertMessageType(::opencensus::trace::exporter::MessageEvent::Type type) {
  using Type = ::opencensus::trace::exporter::MessageEvent::Type;
  switch (type) {
  case Type::SENT:
    return ::opencensus::proto::trace::v1::
        Span_TimeEvent_MessageEvent_Type_SENT;
  case Type::RECEIVED:
    return ::opencensus::proto::trace::v1::
        Span_TimeEvent_MessageEvent_Type_RECEIVED;
  }
  return ::opencensus::proto::trace::v1::
      Span_TimeEvent_MessageEvent_Type_TYPE_UNSPECIFIED;
}

using AttributeMap =
    ::google::protobuf::Map<std::string,
                            ::opencensus::proto::trace::v1::AttributeValue>;

void PopulateAttributes(
    const std::unordered_map<
        std::string, ::opencensus::trace::exporter::AttributeValue> &attributes,
    AttributeMap *attribute_map) {
  for (const auto &attr : attributes) {
    using Type = ::opencensus::trace::exporter::AttributeValue::Type;
    switch (attr.second.type()) {
    case Type::kString:
      SetTruncatableString(attr.second.string_value(), kAttributeStringLen,
                           (*attribute_map)[attr.first].mutable_string_value());
      break;
    case Type::kBool:
      (*attribute_map)[attr.first].set_bool_value(attr.second.bool_value());
      break;
    case Type::kInt:
      (*attribute_map)[attr.first].set_int_value(attr.second.int_value());
      break;
    }
  }
}

void ConvertAttributes(const ::opencensus::trace::exporter::SpanData &span,
                       ::opencensus::proto::trace::v1::Span *proto_span) {
  PopulateAttributes(span.attributes(),
                     proto_span->mutable_attributes()->mutable_attribute_map());
  proto_span->mutable_attributes()->set_dropped_attributes_count(
      span.num_attributes_dropped());
}

void ConvertTimeEvents(const ::opencensus::trace::exporter::SpanData &span,
                       ::opencensus::proto::trace::v1::Span *proto_span) {
  for (const auto &annotation : span.annotations().events()) {
    auto event = proto_span->mutable_time_events()->add_time_event();

    // Encode Timestamp
    EncodeTimestampProto(annotation.timestamp(), event->mutable_time());

    // Populate annotation.
    SetTruncatableString(annotation.event().description(), kAnnotationStringLen,
                         event->mutable_annotation()->mutable_description());
    PopulateAttributes(annotation.event().attributes(),
                       event->mutable_annotation()
                           ->mutable_attributes()
                           ->mutable_attribute_map());
  }

  for (const auto &message : span.message_events().events()) {
    auto event = proto_span->mutable_time_events()->add_time_event();

    // Encode Timestamp
    EncodeTimestampProto(message.timestamp(), event->mutable_time());

    // Populate message event.
    event->mutable_message_event()->set_type(
        ConvertMessageType(message.event().type()));
    event->mutable_message_event()->set_id(message.event().id());
    event->mutable_message_event()->set_uncompressed_size(
        message.event().uncompressed_size());
    event->mutable_message_event()->set_compressed_size(
        message.event().compressed_size());
  }

  proto_span->mutable_time_events()->set_dropped_annotations_count(
      span.annotations().dropped_events_count());
  proto_span->mutable_time_events()->set_dropped_message_events_count(
      span.message_events().dropped_events_count());
}

void ConvertLinks(const ::opencensus::trace::exporter::SpanData &span,
                  ::opencensus::proto::trace::v1::Span *proto_span) {
  proto_span->mutable_links()->set_dropped_links_count(
      span.num_links_dropped());
  for (const auto &span_link : span.links()) {
    auto link = proto_span->mutable_links()->add_link();
    link->set_trace_id(span_link.trace_id().ToHex());
    link->set_span_id(span_link.span_id().ToHex());
    link->set_type(ConvertLinkType(span_link.type()));
    PopulateAttributes(
        span_link.attributes(),
        proto_span->mutable_attributes()->mutable_attribute_map());
  }
}

class Handler : public ::opencensus::trace::exporter::SpanExporter::Handler {
public:
  Handler(OcAgentOptions &&opts);

  void Export(const std::vector<::opencensus::trace::exporter::SpanData> &spans)
      override;

private:
  const OcAgentOptions opts_;
  ::opencensus::proto::agent::common::v1::Node nodeInfo_;
  void ConnectAgent();
  void InitNode();
  void ExportRpcRequest(
      const ::opencensus::proto::agent::trace::v1::ExportTraceServiceRequest &);
};

void ConvertSpans(
    const std::vector<::opencensus::trace::exporter::SpanData> &spans,
    ::opencensus::proto::agent::trace::v1::ExportTraceServiceRequest *request) {
  for (const auto &from_span : spans) {
    auto to_span = request->add_spans();

    // 1. trace_id
    to_span->set_trace_id(from_span.context().trace_id().ToHex());
    // 2. span_id
    to_span->set_span_id(from_span.context().span_id().ToHex());
    // 3. parent_span_id
    to_span->set_parent_span_id(from_span.parent_span_id().ToHex());
    // 4. name
    SetTruncatableString(from_span.name(), kDisplayNameStringLen,
                         to_span->mutable_name());
    // 5. The start time of the span.
    EncodeTimestampProto(from_span.start_time(), to_span->mutable_start_time());
    // 6. The end time of the span.
    EncodeTimestampProto(from_span.end_time(), to_span->mutable_end_time());

    // 7. Export Attributes
    ConvertAttributes(from_span, to_span);

    // 8. stack_trace

    // 9. Export Time Events.
    ConvertTimeEvents(from_span, to_span);

    // 10. Export Links.
    ConvertLinks(from_span, to_span);

    // 11. The status of the span.
    to_span->mutable_status()->set_code(
        static_cast<int32_t>(from_span.status().CanonicalCode()));
    to_span->mutable_status()->set_message(from_span.status().error_message());

    // 12. True if the parent is on a different process.
    to_span->mutable_same_process_as_parent_span()->set_value(
        !from_span.has_remote_parent());

    // 13. child_span_count

    // 14. span kind

    // 15. tracestate

    // 16. resource
    // (TODO) Add FULL support.
  }
}

Handler::Handler(OcAgentOptions &&opts) : opts_(std::move(opts)) {
  InitNode();
  ConnectAgent();
}

void Handler::Export(
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  ::opencensus::proto::agent::trace::v1::ExportTraceServiceRequest request;
  ConvertSpans(spans, &request);
  *request.mutable_node() = nodeInfo_;
  ExportRpcRequest(request);
}

void Handler::ExportRpcRequest(
    const ::opencensus::proto::agent::trace::v1::ExportTraceServiceRequest
        &request) {
  grpc::ClientContext context;
  context.set_deadline(absl::ToChronoTime(absl::Now() + opts_.rpc_deadline));

  auto stream = opts_.trace_service_stub->Export(&context);
  if (stream == nullptr) {
    std::cerr << "Export() got an NULL stream.\n";
    return;
  }

  if (!stream->Write(request)) {
    std::cerr << "Export stream Write() failed.\n";
  }
}

void Handler::InitNode() {
  auto identifier = nodeInfo_.mutable_identifier();

  identifier->set_host_name(::opencensus::common::Hostname());
  identifier->set_pid(getpid());

  EncodeTimestampProto(absl::Now(), identifier->mutable_start_timestamp());

  auto library_info = nodeInfo_.mutable_library_info();
  library_info->set_language(
      ::opencensus::proto::agent::common::v1::LibraryInfo_Language_CPP);
  library_info->set_exporter_version(OPENCENSUS_VERSION);
  library_info->set_core_library_version(OPENCENSUS_VERSION);

  auto service_info = nodeInfo_.mutable_service_info();
  service_info->set_name(::opencensus::common::Hostname());
}

void Handler::ConnectAgent() {
  ::opencensus::proto::agent::trace::v1::ExportTraceServiceRequest request;
  *request.mutable_node() = nodeInfo_;
  ExportRpcRequest(request);

  ::opencensus::proto::agent::trace::v1::CurrentLibraryConfig cur_lib_cfg;
  *cur_lib_cfg.mutable_node() = nodeInfo_;

  // Config
  auto config = cur_lib_cfg.mutable_config();
  auto sampler = config->mutable_constant_sampler();
  sampler->set_decision(::opencensus::proto::trace::v1::
                            ConstantSampler_ConstantDecision_ALWAYS_ON);

  grpc::ClientContext context;
  context.set_deadline(absl::ToChronoTime(absl::Now() + opts_.rpc_deadline));

  auto stream = opts_.trace_service_stub->Config(&context);
  if (stream == nullptr) {
    std::cerr << "Config() got an NULL stream.\n";
    return;
  }

  if (!stream->Write(cur_lib_cfg)) {
    std::cerr << "Config stream Write() failed.\n";
  }
}

} // namespace

// static
void OcAgentExporter::Register(OcAgentOptions &&opts) {
  if (opts.trace_service_stub == nullptr) {
    auto channel = grpc::CreateCustomChannel(
        opts.address, grpc::InsecureChannelCredentials(),
        ::opencensus::common::WithUserAgent());
    opts.trace_service_stub =
        ::opencensus::proto::agent::trace::v1::TraceService::NewStub(channel);
  }
  ::opencensus::trace::exporter::SpanExporter::RegisterHandler(
      absl::make_unique<Handler>(std::move(opts)));
}

} // namespace trace
} // namespace exporters
} // namespace opencensus
