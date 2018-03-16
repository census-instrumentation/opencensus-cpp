#include "zipkin_exporter.h"

#include <thrift/protocol/TBinaryProtocol.h>

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "base64.h"

namespace opencensus {
namespace exporters {
namespace trace {

namespace {

size_t SerializeBinary(apache::thrift::protocol::TBinaryProtocol *protocol,
                       const ::opencensus::trace::exporter::SpanData &span) {
  ::opencensus::zipkin::Span zipkin_span;

  // Populate zipkin span
  zipkin_span.__set_name(std::string(span.name()));
  zipkin_span.__set_trace_id_high(strtoull(
      span.context().trace_id().ToHex().substr(0, 16).c_str(), NULL, 16));
  zipkin_span.__set_trace_id(
      strtoull(span.context().trace_id().ToHex().substr(16).c_str(), NULL, 16));
  zipkin_span.__set_id(
      strtoull(span.context().span_id().ToHex().c_str(), NULL, 16));
  zipkin_span.__set_parent_id(
      strtoull(span.parent_span_id().ToHex().c_str(), NULL, 16));

  // Annotations
  for (const auto &annotation : span.annotations().events()) {
    ::opencensus::zipkin::Annotation zipkin_annotation;
    zipkin_annotation.__set_timestamp(
        absl::ToUnixMicros(annotation.timestamp()));
    zipkin_annotation.__set_value(annotation.event().DebugString());
    zipkin_span.annotations.push_back(zipkin_annotation);
  }

  // Attributes
  for (const auto &attribute : span.attributes()) {
    ::opencensus::zipkin::BinaryAnnotation zipkin_annotation;
    zipkin_annotation.__set_key(attribute.first);
    switch (attribute.second.type()) {
      case ::opencensus::trace::AttributeValueRef::Type::kString:
        zipkin_annotation.__set_value(attribute.second.string_value());
        zipkin_annotation.__set_annotation_type(
            ::opencensus::zipkin::AnnotationType::STRING);
        break;
      case ::opencensus::trace::AttributeValueRef::Type::kBool: {
        bool bool_value = attribute.second.bool_value();
        zipkin_annotation.__set_value(
            std::string(reinterpret_cast<const char *>(&bool_value), 1));
        zipkin_annotation.__set_annotation_type(
            ::opencensus::zipkin::AnnotationType::BOOL);
      } break;
      case ::opencensus::trace::AttributeValueRef::Type::kInt: {
        int64_t int_value = attribute.second.int_value();
        zipkin_annotation.__set_value(
            std::string(reinterpret_cast<const char *>(&int_value), 8));
        zipkin_annotation.__set_annotation_type(
            ::opencensus::zipkin::AnnotationType::I64);
      } break;
      default:
        break;
    }
    zipkin_span.binary_annotations.push_back(zipkin_annotation);
  }

  zipkin_span.__set_timestamp(absl::ToUnixMicros(span.start_time()));
  zipkin_span.__set_duration(
      absl::ToInt64Microseconds(span.end_time() - span.start_time()));

  return zipkin_span.write(protocol);
}

void SerializeJson(rapidjson::Writer<rapidjson::StringBuffer> *writer,
                   const ::opencensus::trace::exporter::SpanData &span) {
  writer->StartObject();

  writer->Key("name");
  writer->String(std::string(span.name()));

  writer->Key("traceId");
  writer->String(span.context().trace_id().ToHex());

  writer->Key("id");
  writer->String(span.context().span_id().ToHex());

  writer->Key("parentId");
  writer->String(span.parent_span_id().ToHex());

  writer->Key("annotations");
  writer->StartArray();
  for (const auto &annotation : span.annotations().events()) {
    writer->StartObject();
    writer->Key("timestamp");
    writer->Int64(absl::ToUnixMicros(annotation.timestamp()));
    writer->Key("value");
    writer->String(annotation.event().DebugString());
    writer->EndObject();
  }
  writer->EndArray(span.annotations().events().size());

  writer->Key("binaryAnnotations");
  writer->StartArray();
  for (const auto &attribute : span.attributes()) {
    writer->StartObject();
    writer->Key("key");
    writer->String(attribute.first);
    writer->Key("value");
    switch (attribute.second.type()) {
      case ::opencensus::trace::AttributeValueRef::Type::kString:
        writer->String(attribute.second.string_value());
        writer->Key("type");
        writer->String("STRING");
        break;
      case ::opencensus::trace::AttributeValueRef::Type::kBool:
        writer->Bool(attribute.second.bool_value());
        writer->Key("type");
        writer->String("BOOL");
        break;
      case ::opencensus::trace::AttributeValueRef::Type::kInt:
        writer->Int64(attribute.second.int_value());
        writer->Key("type");
        writer->String("I64");
        break;
      default:
        break;
    }
    writer->EndObject();
  }
  writer->EndArray(span.attributes().size());

  writer->Key("timestamp");
  writer->Int64(absl::ToUnixMicros(span.start_time()));

  writer->Key("duration");
  writer->Int64(absl::ToInt64Microseconds(span.end_time() - span.start_time()));

  writer->EndObject();
}

}  // namespace

size_t BinaryCodec::Encode(
    std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf,
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  apache::thrift::protocol::TBinaryProtocol protocol(buf);

  size_t wrote =
      protocol.writeListBegin(apache::thrift::protocol::T_STRUCT, spans.size());
  for (auto &span : spans) {
    wrote += SerializeBinary(&protocol, span);
  }

  return wrote + protocol.writeListEnd();
}

size_t JsonCodec::Encode(
    std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf,
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartArray();
  for (auto &span : spans) {
    SerializeJson(&writer, span);
  }
  writer.EndArray();
  buf->write((const uint8_t *)buffer.GetString(), buffer.GetSize());

  return buffer.GetSize();
}

size_t PrettyJsonCodec::Encode(
    std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf,
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

  writer.StartArray();
  for (auto &span : spans) {
    SerializeJson(&writer, span);
  }
  writer.EndArray();
  buf->write((const uint8_t *)buffer.GetString(), buffer.GetSize());

  return buffer.GetSize();
}

struct ScribeOptions {
  ScribeOptions(const std::string &h, int16_t p) : host(h), port(p) {}
  ScribeOptions(folly::Uri &uri);

  // Server address
  std::string host;
  // Server port
  int16_t port;
  // The Scribe category used to transmit the spans.
  // The default category is zipkin.
  std::string category = "zipkin";
  // The maximum retry times.
  // The default maximum retry times is 3.
  size_t max_retry_times = 3;
};

class Scribe : public ::opencensus::exporters::trace::TraceClient {
 public:
  Scribe(const ScribeOptions &options)
      : options_(options.host, options.port),
        socket_(
            new apache::thrift::transport::TSocket(options.host, options.port)),
        transport_(new apache::thrift::transport::TFramedTransport(socket_)),
        protocol_(new apache::thrift::protocol::TBinaryProtocol(transport_)),
        client_(new ::opencensus::scribe::ScribeClient(protocol_)) {}

  const ScribeOptions Options() const { return options_; };

  virtual void SendMessage(const uint8_t *msg, size_t size) override;

 private:
  ScribeOptions options_;
  std::shared_ptr<apache::thrift::transport::TSocket> socket_;
  std::shared_ptr<apache::thrift::transport::TFramedTransport> transport_;
  std::shared_ptr<apache::thrift::protocol::TBinaryProtocol> protocol_;
  std::shared_ptr<::opencensus::scribe::ScribeClient> client_;

  bool Connected() const;
  bool Reconnect();
};

ScribeOptions::ScribeOptions(folly::Uri &uri) {
  std::vector<folly::StringPiece> parts;

  host = folly::toStdString(uri.host());
  if (uri.port()) port = uri.port();
  folly::split("/", uri.path(), parts);
  if (parts.size() > 1) category = parts[1].str();

  for (auto &param : uri.getQueryParams()) {
    if (param.first == "max_retry_times") {
      max_retry_times = folly::to<size_t>(param.second);
    }
  }
}

void Scribe::SendMessage(const uint8_t *msg, size_t size) {
  ::opencensus::scribe::LogEntry entry;
  entry.__set_category(Options().category);
  entry.__set_message(base64::Encode(msg, size));

  std::vector<::opencensus::scribe::LogEntry> entries;
  entries.push_back(entry);

  ::opencensus::scribe::ResultCode::type res =
      ::opencensus::scribe::ResultCode::type::TRY_LATER;
  size_t retry_times = 0;

  do {
    if (Connected() || Reconnect()) {
      res = client_->Log(entries);
    }
  } while (res == ::opencensus::scribe::ResultCode::type::TRY_LATER &&
           retry_times++ < Options().max_retry_times);
}

bool Scribe::Connected() const { return socket_->isOpen(); }

bool Scribe::Reconnect() {
  socket_->open();
  return socket_->isOpen();
}

void ZipkinExporter::Register(const ZipkinExporterOptions &options) {
  // Create new exporter.
  ZipkinExporter *exporter = new ZipkinExporter(options);

  if (exporter->uri_.scheme() == "scribe" ||
      exporter->uri_.scheme() == "thrift") {
    // Create trace client handler that will send encoded span information to
    // collection server.
    ScribeOptions scribe_options(exporter->uri_);
    exporter->trace_client_ = std::unique_ptr<TraceClient>(
        dynamic_cast<TraceClient *>(new Scribe(scribe_options)));
  }

  ::opencensus::trace::exporter::SpanExporter::RegisterHandler(
      absl::WrapUnique<::opencensus::trace::exporter::SpanExporter::Handler>(
          exporter));
}

void ZipkinExporter::Export(
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  if (!spans.empty()) {
    std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf(
        new apache::thrift::transport::TMemoryBuffer());

    message_codec_->Encode(buf, spans);
    uint8_t *msg = nullptr;
    uint32_t size = 0;
    buf->getBuffer(&msg, &size);
    trace_client_->SendMessage(msg, size);
  }
}

void ZipkinExporter::ExportForTesting(
    const ZipkinExporterOptions &options,
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  if (!spans.empty()) {
    // Create new exporter.
    ZipkinExporter *exporter = new ZipkinExporter(options);

    std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf(
        new apache::thrift::transport::TMemoryBuffer());

    exporter->message_codec_->Encode(buf, spans);
    uint8_t *msg = nullptr;
    uint32_t size = 0;
    buf->getBuffer(&msg, &size);

    std::string str(reinterpret_cast<char *>(msg), size);
    fprintf(stderr, "%s\n", str.c_str());
  }
}

}  // namespace trace
}  // namespace exporters
}  // namespace opencensus
