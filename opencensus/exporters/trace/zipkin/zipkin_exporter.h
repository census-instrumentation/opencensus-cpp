#ifndef OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_ZIPKIN_EXPORTER_H_
#define OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_ZIPKIN_EXPORTER_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include <folly/Uri.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

#include "absl/memory/memory.h"
#include "opencensus/trace/exporter/span_exporter.h"
#include "opencensus/trace/span.h"
#include "scribe.h"
#include "zipkin_core_types.h"

namespace opencensus {

class BinaryCodec;
class JsonCodec;
class PrettyJsonCodec;

// MessageCodec is used for encoding message sets.
class MessageCodec {
 public:
  virtual ~MessageCodec() = default;

  virtual const std::string name() const = 0;

  virtual const std::string mime_type() const = 0;

  virtual size_t Encode(
      std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf,
      const std::vector<::opencensus::trace::exporter::SpanData> &spans) = 0;
};

// Thrift binary encoding
class BinaryCodec : public MessageCodec {
 public:
  virtual const std::string name() const override { return "binary"; }

  virtual const std::string mime_type() const override {
    return "application/x-thrift";
  }

  virtual size_t Encode(
      std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf,
      const std::vector<::opencensus::trace::exporter::SpanData> &spans)
      override;
};

// JSON encoding
class JsonCodec : public MessageCodec {
 public:
  virtual const std::string name() const override { return "json"; }

  virtual const std::string mime_type() const override {
    return "application/json";
  }

  virtual size_t Encode(
      std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf,
      const std::vector<::opencensus::trace::exporter::SpanData> &spans)
      override;
};

// Pretty print JSON encoding
class PrettyJsonCodec : public MessageCodec {
 public:
  virtual const std::string name() const override { return "pretty_json"; }

  virtual const std::string mime_type() const override {
    return "application/json";
  }

  virtual size_t Encode(
      std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf,
      const std::vector<::opencensus::trace::exporter::SpanData> &spans)
      override;
};

class TraceClient {
 public:
  virtual void SendMessage(const uint8_t *msg, size_t size) = 0;
};

struct ZipkinExporterOptions {
  ZipkinExporterOptions()
      : host("::1"),
        port(3000),
        codec_type(ZipkinExporterOptions::CodecType::kJson),
        exporter_type(ZipkinExporterOptions::ExporterType::kScribeExporter) {}

  // Uniform Resource Identifier for server that spans will be sent to.
  absl::string_view uri;
  // Server address
  std::string host;
  // Server port
  int16_t port;
  // Message codec to use for encoding message sets.
  // Default = Json
  enum class CodecType : uint8_t { kBinary, kJson, kPrettyJson };
  CodecType codec_type;

  enum class ExporterType : uint8_t { kScribeExporter, kHttpExporter };
  ExporterType exporter_type;

  static std::unique_ptr<MessageCodec> GetCodec(CodecType type) {
    switch (type) {
      case ZipkinExporterOptions::CodecType::kBinary:
        return std::unique_ptr<MessageCodec>(
            dynamic_cast<MessageCodec *>(new BinaryCodec));
      case ZipkinExporterOptions::CodecType::kJson:
        return std::unique_ptr<MessageCodec>(
            dynamic_cast<MessageCodec *>(new JsonCodec));
      case ZipkinExporterOptions::CodecType::kPrettyJson:
        return std::unique_ptr<MessageCodec>(
            dynamic_cast<MessageCodec *>(new PrettyJsonCodec));
    }
    return nullptr;
  }
};

class ZipkinExporter
    : public ::opencensus::trace::exporter::SpanExporter::Handler {
 public:
  void Export(const std::vector<::opencensus::trace::exporter::SpanData> &spans)
      override;
  static void Register(const ZipkinExporterOptions &options);

 private:
  ZipkinExporter(const ZipkinExporterOptions &options)
      : uri_(options.uri),
        uri_str_(options.uri),
        codec_type_(options.codec_type),
        message_codec_(ZipkinExporterOptions::GetCodec(options.codec_type)),
        exporter_type_(options.exporter_type) {}

  folly::Uri uri_;
  const std::string uri_str_;
  ZipkinExporterOptions::CodecType codec_type_;
  std::unique_ptr<MessageCodec> message_codec_;
  ZipkinExporterOptions::ExporterType exporter_type_;
  std::unique_ptr<TraceClient> trace_client_;
};

}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_ZIPKIN_EXPORTER_H_
