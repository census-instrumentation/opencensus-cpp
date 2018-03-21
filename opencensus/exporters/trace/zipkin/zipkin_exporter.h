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

#ifndef OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_ZIPKIN_EXPORTER_H_
#define OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_ZIPKIN_EXPORTER_H_

#include <memory>
#include <string>

#include "absl/memory/memory.h"
#include "absl/time/time.h"
#include "opencensus/trace/exporter/span_exporter.h"
#include "opencensus/trace/span.h"

namespace opencensus {
namespace exporters {
namespace trace {

// MessageCodec is used for encoding message sets.
class MessageCodec {
 public:
  virtual ~MessageCodec() = default;

  virtual const std::string name() const = 0;

  virtual std::string Encode(
      const std::vector<::opencensus::trace::exporter::SpanData> &spans) = 0;
};

// JSON encoding
class JsonCodec : public MessageCodec {
 public:
  const std::string name() const override { return "json"; }

  std::string Encode(const std::vector<::opencensus::trace::exporter::SpanData>
                         &spans) override;
};

// Pretty print JSON encoding
class PrettyJsonCodec : public MessageCodec {
 public:
  const std::string name() const override { return "pretty_json"; }

  std::string Encode(const std::vector<::opencensus::trace::exporter::SpanData>
                         &spans) override;
};

class ExportClient {
 public:
  virtual void SendMessage(const std::string &msg, size_t size) = 0;
};

struct ZipkinExporterOptions {
  ZipkinExporterOptions(absl::string_view url)
      : url(url), codec_type(ZipkinExporterOptions::CodecType::kJson) {}

  // Uniform Resource Location for server that spans will be sent to.
  absl::string_view url;
  // The proxy to use for the upcoming request.
  std::string proxy;
  // Tunnel through HTTP proxy
  bool http_proxy_tunnel = false;
  // The maximum number of redirects allowed. The default maximum redirect times
  // is 3.
  size_t max_redirect_times = 3;
  // The maximum timeout for TCP connect. The default connect timeout is 5
  // seconds.
  absl::Duration connect_timeout = absl::Seconds(5);
  // The maximum timeout for HTTP request. The default request timeout is 15
  // seconds.
  absl::Duration request_timeout = absl::Seconds(15);

  // Message codec to use for encoding message sets.
  // Default = Json
  enum class CodecType : uint8_t { kJson, kPrettyJson };
  CodecType codec_type;

  std::unique_ptr<MessageCodec> GetCodec() const {
    switch (codec_type) {
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
  friend class ZipkinExporterTestPeer;

  static void ExportForTesting(
      const ZipkinExporterOptions &options,
      const std::vector<::opencensus::trace::exporter::SpanData> &spans);

  ZipkinExporter(const ZipkinExporterOptions &options)
      : options_(options),
        codec_type_(options.codec_type),
        message_codec_(options.GetCodec()) {}
  ~ZipkinExporter() {}

  ZipkinExporterOptions options_;
  ZipkinExporterOptions::CodecType codec_type_;
  std::unique_ptr<MessageCodec> message_codec_;
  std::unique_ptr<ExportClient> trace_client_;
};

}  // namespace trace
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_TRACE_ZIPKIN_ZIPKIN_EXPORTER_H_
