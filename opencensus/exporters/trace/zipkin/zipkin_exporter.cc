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

#include "zipkin_exporter.h"

#include <curl/curl.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include "absl/strings/str_cat.h"

namespace opencensus {
namespace exporters {
namespace trace {

namespace {

constexpr const char *kZipkinLibname = "zipkin";
constexpr const char *kZkipkinVersion = "2.0";

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

class CurlEnv {
 public:
  CurlEnv() { curl_global_init(CURL_GLOBAL_DEFAULT); }
  ~CurlEnv() { curl_global_cleanup(); }
};

}  // namespace

std::string JsonCodec::Encode(
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartArray();
  for (auto &span : spans) {
    SerializeJson(&writer, span);
  }
  writer.EndArray();
  return buffer.GetString();
}

std::string PrettyJsonCodec::Encode(
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

  writer.StartArray();
  for (auto &span : spans) {
    SerializeJson(&writer, span);
  }
  writer.EndArray();
  return buffer.GetString();
}

struct HttpClientOptions {
  HttpClientOptions(absl::string_view u) : url(u) {}

  // The URL to use in the request.
  std::string url;
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
};

class HttpClient : public ExportClient {
 public:
  HttpClient(const HttpClientOptions &options)
      : url_(options.url),
        proxy_(options.proxy),
        http_proxy_tunnel_(options.http_proxy_tunnel),
        max_redirect_times_(options.max_redirect_times),
        connect_timeout_(options.connect_timeout),
        request_timeout_(options.request_timeout) {}

  void SendMessage(const std::string &msg, size_t size) override;

 private:
  std::string url_;
  std::string proxy_;
  bool http_proxy_tunnel_;
  size_t max_redirect_times_;
  absl::Duration connect_timeout_;
  absl::Duration request_timeout_;
};

void HttpClient::SendMessage(const std::string &msg, size_t size) {
  CURLcode res;
  struct curl_slist *headers = nullptr;
  char content_type[128] = {0}, err_msg[CURL_ERROR_SIZE] = {0};
  std::string error;
  CURL *curl = curl_easy_init();

  if (!curl) {
    // Failed to create curl handle.
    return;
  }

  const std::string mime_type = "application/json";
  snprintf(content_type, sizeof(content_type), "Content-Type: %s",
           mime_type.c_str());

  headers = curl_slist_append(headers, content_type);
  headers = curl_slist_append(headers, "Expect:");
  const uint8_t *data = reinterpret_cast<const uint8_t *>(msg.data());

  if (CURLE_OK !=
      (res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_msg))) {
    // Failed to set curl error buffer.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK !=
             (res = curl_easy_setopt(curl, CURLOPT_URL, url_.c_str()))) {
    // Failed to set url.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK !=
             (res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers))) {
    // Failed to set http header.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK != (res = curl_easy_setopt(
                              curl, CURLOPT_USERAGENT,
                              absl::StrCat(kZipkinLibname, "/", kZkipkinVersion)
                                  .c_str()))) {
    // Failed to set http user agent.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK !=
             (res = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size))) {
    // Failed to set http body size.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK !=
             (res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data))) {
    // Failed to set http body data.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK !=
             curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,
                              absl::ToInt64Milliseconds(connect_timeout_))) {
    // Failed to set connect timeout.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK !=
             curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS,
                              absl::ToInt64Milliseconds(request_timeout_))) {
    // Failed to set request timeout.
    error = curl_easy_strerror(res);
  } else if (CURLE_OK != (res = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1))) {
    // Failed to disable signals.
    error = curl_easy_strerror(res);
  } else {
    if (proxy_.empty()) {
      if (CURLE_OK !=
          (res = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_.c_str()))) {
        // Failed to set proxy.
        error = curl_easy_strerror(res);
      }

      if (http_proxy_tunnel_) {
        if (CURLE_OK !=
            (res = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP))) {
          // Failed to set HTTP proxy type.
          error = curl_easy_strerror(res);
        } else if (CURLE_OK !=
                   (res = curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1))) {
          // Failed to set HTTP proxy tunnel.
          error = curl_easy_strerror(res);
        }
      }
    }

    if (max_redirect_times_) {
      if (CURLE_OK !=
          (res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1))) {
        // Failed to enable follow location.
        error = curl_easy_strerror(res);
      } else if (CURLE_OK != (res = curl_easy_setopt(curl, CURLOPT_MAXREDIRS,
                                                     max_redirect_times_))) {
        // Failed to set max redirect times.
        error = curl_easy_strerror(res);
      }
    }

    // Sending HTTP request to url.
    if (CURLE_OK != (res = curl_easy_perform(curl))) {
      // Failed to send http request.
      error = curl_easy_strerror(res);
    } else {
      long status_code = 0;
      double total_time = 0, uploaded_bytes = 0, upload_speed = 0;

      if (CURLE_OK != (res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
                                               &status_code))) {
        // Failed to get status code.
        error = curl_easy_strerror(res);
      } else if (CURLE_OK != (res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME,
                                                      &total_time))) {
        // Failed to get total time.
        error = curl_easy_strerror(res);
      } else if (CURLE_OK !=
                 (res = curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD,
                                          &uploaded_bytes))) {
        // Failed to get uploaded bytes.
        error = curl_easy_strerror(res);
      } else if (CURLE_OK !=
                 (res = curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD,
                                          &upload_speed))) {
        // Failed to get upload speed.
        error = curl_easy_strerror(res);
      } else {
        // LOG(INFO) << "HTTP request finished, status " << status_code
        //           << ", uploaded " << uploaded_bytes << " bytes in "
        //           << total_time << " seconds (" << (upload_speed / 1024)
        //           << " KB/s)";
      }
    }
  }

  // TODO: Log error if one exists.
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
}

void ZipkinExporter::Register(const ZipkinExporterOptions &options) {
  // Create new exporter.
  static CurlEnv *curl_lib = new CurlEnv();
  ZipkinExporter *exporter = new ZipkinExporter(options);

  // Create client handler that will send encoded span information to
  // collection server.
  HttpClientOptions http_client_options(exporter->options_.url);
  exporter->trace_client_ = std::unique_ptr<ExportClient>(
      dynamic_cast<ExportClient *>(new HttpClient(http_client_options)));

  ::opencensus::trace::exporter::SpanExporter::RegisterHandler(
      absl::WrapUnique<::opencensus::trace::exporter::SpanExporter::Handler>(
          exporter));
}

void ZipkinExporter::Export(
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  if (!spans.empty()) {
    std::string msg = message_codec_->Encode(spans);
    trace_client_->SendMessage(msg, msg.size());
  }
}

void ZipkinExporter::ExportForTesting(
    const ZipkinExporterOptions &options,
    const std::vector<::opencensus::trace::exporter::SpanData> &spans) {
  if (!spans.empty()) {
    // Create new exporter.
    ZipkinExporter *exporter = new ZipkinExporter(options);
    std::string msg = exporter->message_codec_->Encode(spans);
    fprintf(stderr, "%s\n", msg.c_str());
  }
}

}  // namespace trace
}  // namespace exporters
}  // namespace opencensus
