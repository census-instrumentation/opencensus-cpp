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

#include "opencensus/trace/internal/span_exporter_impl.h"

#include <utility>

#include "absl/synchronization/mutex.h"
#include "opencensus/trace/exporter/span_data.h"
#include "opencensus/trace/exporter/span_exporter.h"

namespace opencensus {
namespace trace {
namespace exporter {

SpanExporterImpl* SpanExporterImpl::span_exporter_ = nullptr;

SpanExporterImpl* SpanExporterImpl::Get() {
  static SpanExporterImpl* span_exporter_impl = new SpanExporterImpl(
      kDefaultBufferSize, absl::Milliseconds(kIntervalWaitTimeInMillis));
  return span_exporter_impl;
}

// Create detached worker thread
SpanExporterImpl::SpanExporterImpl(uint32_t buffer_size,
                                   absl::Duration interval)
    : buffer_size_(buffer_size),
      interval_(interval),
      size_(0),
      t_(&SpanExporterImpl::RunWorkerLoop, this) {}

void SpanExporterImpl::Register(
    std::unique_ptr<SpanExporter::Handler> handler) {
  absl::MutexLock lock(&handler_mu_);
  handlers_.emplace_back(std::move(handler));
}

void SpanExporterImpl::AddSpan(
    const std::shared_ptr<opencensus::trace::SpanImpl>& span_impl) {
  absl::MutexLock l(&span_mu_);
  spans_.emplace_back(span_impl);
  size_.fetch_add(1, std::memory_order_acq_rel);
}

void SpanExporterImpl::RunWorkerLoop() {
  std::vector<opencensus::trace::exporter::SpanData> span_data_;
  std::vector<std::shared_ptr<opencensus::trace::SpanImpl>> spans_copy_;
  // Thread loops forever.
  // TODO: Add in shutdown mechanism.
  while (true) {
    {
      absl::MutexLock l(&span_mu_);
      // Wait until batch is full or interval time has been exceeded.
      span_mu_.AwaitWithTimeout(
          absl::Condition(
              +[](SpanExporterImpl* ptr) {
                return (ptr->size_.load(std::memory_order_acquire) >=
                        ptr->buffer_size_);
              },
              this),
          interval_);
      if (spans_.empty()) {
        continue;
      }
      std::swap(spans_copy_, spans_);
      size_.store(0, std::memory_order_release);
    }
    for (const auto& span : spans_copy_) {
      span_data_.emplace_back(span->ToSpanData());
    }
    Export(span_data_);
    spans_copy_.clear();
    span_data_.clear();
  }
}

void SpanExporterImpl::Export(const std::vector<SpanData>& span_data) {
  // Call each registered handler.
  absl::MutexLock lock(&handler_mu_);
  for (const auto& handler : handlers_) {
    handler->Export(span_data);
  }
}

}  // namespace exporter
}  // namespace trace
}  // namespace opencensus
