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

#include "opencensus/stats/stats_exporter.h"

#include <thread>  // NOLINT
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"

namespace opencensus {
namespace stats {

class StatsExporterImpl {
 public:
  static StatsExporterImpl* Get() {
    static StatsExporterImpl* global_stats_exporter_impl =
        new StatsExporterImpl();
    return global_stats_exporter_impl;
  }

  void AddView(const ViewDescriptor& view) {
    absl::MutexLock l(&mu_);
    views_[view.name()] = absl::make_unique<opencensus::stats::View>(view);
  }

  void RemoveView(absl::string_view name) {
    absl::MutexLock l(&mu_);
    views_.erase(std::string(name));
  }

  // Adds a handler, which cannot be subsequently removed (except by
  // ClearHandlersForTesting()). The background thread is started when the
  // first handler is registered.
  void RegisterPushHandler(std::unique_ptr<StatsExporter::Handler> handler) {
    absl::MutexLock l(&mu_);
    handlers_.push_back(std::move(handler));
    if (!thread_started_) {
      StartExportThread();
    }
  }

  std::vector<std::pair<ViewDescriptor, ViewData>> GetViewData() {
    absl::ReaderMutexLock l(&mu_);
    std::vector<std::pair<ViewDescriptor, ViewData>> data;
    data.reserve(views_.size());
    for (const auto& view : views_) {
      data.emplace_back(view.second->descriptor(), view.second->GetData());
    }
    return data;
  }

  void Export() {
    absl::ReaderMutexLock l(&mu_);
    for (const auto& view : views_) {
      SendToHandlers(view.second->descriptor(), view.second->GetData());
    }
  }

  void ClearHandlersForTesting() {
    absl::MutexLock l(&mu_);
    handlers_.clear();
  }

 private:
  StatsExporterImpl() {}

  void SendToHandlers(const ViewDescriptor& descriptor, const ViewData& data)
      SHARED_LOCKS_REQUIRED(mu_) {
    for (auto& handler : handlers_) {
      handler->ExportViewData(descriptor, data);
    }
  }

  void StartExportThread() EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    t_ = std::thread(&StatsExporterImpl::RunWorkerLoop, this);
    thread_started_ = true;
  }

  // Loops forever, calling Export() every export_interval_.
  void RunWorkerLoop() {
    absl::Time next_export_time = absl::Now() + export_interval_;
    while (true) {
      // SleepFor() returns immediately when given a negative duration.
      absl::SleepFor(next_export_time - absl::Now());
      // In case the last export took longer than the export interval, we
      // calculate the next time from now.
      next_export_time = absl::Now() + export_interval_;
      Export();
    }
  }

  const absl::Duration export_interval_ = absl::Seconds(10);

  mutable absl::Mutex mu_;

  std::vector<std::unique_ptr<StatsExporter::Handler>> handlers_
      GUARDED_BY(mu_);
  std::unordered_map<std::string, std::unique_ptr<View>> views_ GUARDED_BY(mu_);

  bool thread_started_ GUARDED_BY(mu_) = false;
  std::thread t_ GUARDED_BY(mu_);
};

void StatsExporter::AddView(const ViewDescriptor& view) {
  StatsExporterImpl::Get()->AddView(view);
}

void StatsExporter::RemoveView(absl::string_view name) {
  StatsExporterImpl::Get()->RemoveView(name);
}

void StatsExporter::RegisterPushHandler(std::unique_ptr<Handler> handler) {
  StatsExporterImpl::Get()->RegisterPushHandler(std::move(handler));
}

std::vector<std::pair<ViewDescriptor, ViewData>> StatsExporter::GetViewData() {
  return StatsExporterImpl::Get()->GetViewData();
}

void StatsExporter::ExportForTesting() { StatsExporterImpl::Get()->Export(); }

void StatsExporter::ClearHandlersForTesting() {
  StatsExporterImpl::Get()->ClearHandlersForTesting();
}

}  // namespace stats
}  // namespace opencensus
