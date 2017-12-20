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

  void RegisterHandler(std::unique_ptr<StatsExporter::Handler> handler) {
    absl::MutexLock l(&mu_);
    handlers_.push_back(std::move(handler));
  }

  void Export() {
    absl::MutexLock l(&mu_);
    for (const auto& view : views_) {
      SendToHandlers(view.second->descriptor(), view.second->GetData());
    }
  }

  void ClearHandlersForTesting() {
    absl::MutexLock l(&mu_);
    handlers_.clear();
  }

 private:
  StatsExporterImpl() : t_(&StatsExporterImpl::RunWorkerLoop, this) {}

  void SendToHandlers(const ViewDescriptor& descriptor, const ViewData& data)
      SHARED_LOCKS_REQUIRED(mu_) {
    for (auto& handler : handlers_) {
      handler->ExportViewData(descriptor, data);
    }
  }

  // Loops forever, calling Export() every export_interval_.
  void RunWorkerLoop() {
    while (true) {
      absl::SleepFor(export_interval_);
      Export();
    }
  }

  const absl::Duration export_interval_ = absl::Seconds(10);

  mutable absl::Mutex mu_;

  std::vector<std::unique_ptr<StatsExporter::Handler>> handlers_
      GUARDED_BY(mu_);
  std::unordered_map<std::string, std::unique_ptr<View>> views_ GUARDED_BY(mu_);
  std::thread t_;
};

void StatsExporter::AddView(const ViewDescriptor& view) {
  StatsExporterImpl::Get()->AddView(view);
}

void StatsExporter::RemoveView(absl::string_view name) {
  StatsExporterImpl::Get()->RemoveView(name);
}

void StatsExporter::RegisterHandler(std::unique_ptr<Handler> handler) {
  StatsExporterImpl::Get()->RegisterHandler(std::move(handler));
}

void StatsExporter::ExportForTesting() { StatsExporterImpl::Get()->Export(); }

void StatsExporter::ClearHandlersForTesting() {
  StatsExporterImpl::Get()->ClearHandlersForTesting();
}

}  // namespace stats
}  // namespace opencensus
