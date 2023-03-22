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

#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"

#include <grpcpp/grpcpp.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "google/monitoring/v3/metric_service.grpc.pb.h"
#include "google/protobuf/empty.pb.h"
#include "opencensus/common/internal/grpc/status.h"
#include "opencensus/common/internal/grpc/with_user_agent.h"
#include "opencensus/common/internal/hostname.h"
#include "opencensus/exporters/stats/stackdriver/internal/stackdriver_utils.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

namespace {

constexpr char kGoogleStackdriverStatsAddress[] = "monitoring.googleapis.com";
constexpr char kProjectIdPrefix[] = "projects/";
// Stackdriver limits a single CreateTimeSeries request to 200 series.
constexpr int kTimeSeriesBatchSize = 200;
constexpr char kDefaultMetricNamePrefix[] = "custom.googleapis.com/opencensus/";

class Handler : public ::opencensus::stats::StatsExporter::Handler {
 public:
  explicit Handler(StackdriverOptions&& opts);

  void ExportViewData(
      const std::vector<std::pair<opencensus::stats::ViewDescriptor,
                                  opencensus::stats::ViewData>>& data)
      ABSL_LOCKS_EXCLUDED(mu_) override;

 private:
  // Registers 'descriptor' with Stackdriver if no view by that name has been
  // registered by this, and adds it to registered_descriptors_ if successful.
  // Returns true if the view has already been registered or registration is
  // successful, and false if the registration fails or the name has already
  // been registered with different parameters.
  bool MaybeRegisterView(const opencensus::stats::ViewDescriptor& descriptor,
                         bool add_task_label)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

  const StackdriverOptions opts_;
  mutable absl::Mutex mu_;
  std::unordered_map<std::string, opencensus::stats::ViewDescriptor>
      registered_descriptors_ ABSL_GUARDED_BY(mu_);
};

StackdriverOptions SetOptionDefaults(StackdriverOptions&& o) {
  StackdriverOptions opts(std::move(o));

  // Prepend prefix because we always use this with a prefix.
  opts.project_id = absl::StrCat(kProjectIdPrefix, opts.project_id);

  if (opts.opencensus_task.empty()) {
    opts.opencensus_task = opencensus::common::OpenCensusTask();
  }

  if (opts.metric_name_prefix.empty()) {
    opts.metric_name_prefix = kDefaultMetricNamePrefix;
  }

  if (opts.metric_service_stub == nullptr) {
    opts.metric_service_stub = google::monitoring::v3::MetricService::NewStub(
        ::grpc::CreateCustomChannel(kGoogleStackdriverStatsAddress,
                                    ::grpc::GoogleDefaultCredentials(),
                                    ::opencensus::common::WithUserAgent()));
  }
  return opts;
}

Handler::Handler(StackdriverOptions&& opts)
    : opts_(SetOptionDefaults(std::move(opts))) {}

void Handler::ExportViewData(
    const std::vector<std::pair<opencensus::stats::ViewDescriptor,
                                opencensus::stats::ViewData>>& data) {
  // TODO: refactor to avoid copying the timeseries.
  absl::MutexLock l(&mu_);
  std::vector<google::monitoring::v3::TimeSeries> time_series;
  for (const auto& datum : data) {
    const opencensus::stats::ViewDescriptor& descriptor = datum.first;
    const std::string metric_type =
        MakeType(opts_.metric_name_prefix, descriptor.name());
    const google::api::MonitoredResource* monitored_resource_for_view =
        MonitoredResourceForView(descriptor, opts_.monitored_resource,
                                 opts_.per_metric_monitored_resource);

    const bool is_known_custom_metric = IsKnownCustomMetric(metric_type);

    // If this is a custom metric, add the opencensus_task label so that
    // different processes produce different timeseries instead of colliding.
    const bool add_task_label = is_known_custom_metric;

    // Builtin metrics are already defined, skip registration.
    if (is_known_custom_metric) {
      // If the view can't be registered, skip it.
      if (!MaybeRegisterView(descriptor, add_task_label)) {
        continue;
      }
    }

    const auto view_time_series = MakeTimeSeries(
        opts_.metric_name_prefix, monitored_resource_for_view, descriptor,
        /*data=*/datum.second, add_task_label, opts_.opencensus_task);
    time_series.insert(time_series.end(), view_time_series.begin(),
                       view_time_series.end());
  }

  const int num_rpcs =
      ceil(static_cast<double>(time_series.size()) / kTimeSeriesBatchSize);

  std::vector<grpc::Status> status(num_rpcs);
  std::vector<grpc::ClientContext> ctx(num_rpcs);
  // We can safely re-use an empty response--it is never updated.
  google::protobuf::Empty response;
  grpc::CompletionQueue cq;

  for (int rpc_index = 0; rpc_index < num_rpcs; ++rpc_index) {
    auto request = google::monitoring::v3::CreateTimeSeriesRequest();
    request.set_name(opts_.project_id);
    const int batch_end = std::min(static_cast<int>(time_series.size()),
                                   (rpc_index + 1) * kTimeSeriesBatchSize);
    for (int i = rpc_index * kTimeSeriesBatchSize; i < batch_end; ++i) {
      *request.add_time_series() = time_series[i];
    }
    ctx[rpc_index].set_deadline(
        absl::ToChronoTime(absl::Now() + opts_.rpc_deadline));
    opts_.prepare_client_context(&ctx[rpc_index]);
    auto rpc(opts_.metric_service_stub->AsyncCreateTimeSeries(&ctx[rpc_index],
                                                              request, &cq));
    rpc->Finish(&response, &status[rpc_index], (void*)(uintptr_t)rpc_index);
  }

  cq.Shutdown();
  void* tag;
  bool ok;
  while (cq.Next(&tag, &ok)) {
    if (ok) {
      const auto& s = status[(uintptr_t)tag];
      if (!s.ok()) {
        std::cerr << "CreateTimeSeries request failed (" << num_rpcs
                  << " RPCs, " << data.size() << " views, "
                  << time_series.size()
                  << " timeseries): " << opencensus::common::ToString(s)
                  << "\n";
      }
    }
  }
}

bool Handler::MaybeRegisterView(
    const opencensus::stats::ViewDescriptor& descriptor, bool add_task_label) {
  const auto& it = registered_descriptors_.find(descriptor.name());
  if (it != registered_descriptors_.end()) {
    if (it->second != descriptor) {
      std::cerr << "Not exporting altered view: " << descriptor.DebugString()
                << "\nAlready registered as: " << it->second.DebugString()
                << "\n";
      return false;
    }
    return true;
  }

  auto request = google::monitoring::v3::CreateMetricDescriptorRequest();
  request.set_name(opts_.project_id);
  SetMetricDescriptor(opts_.project_id, opts_.metric_name_prefix, descriptor,
                      add_task_label, request.mutable_metric_descriptor());
  ::grpc::ClientContext context;
  context.set_deadline(absl::ToChronoTime(absl::Now() + opts_.rpc_deadline));
  google::api::MetricDescriptor response;
  opts_.prepare_client_context(&context);
  ::grpc::Status status = opts_.metric_service_stub->CreateMetricDescriptor(
      &context, request, &response);
  if (!status.ok()) {
    std::cerr << "CreateMetricDescriptor(" << request.metric_descriptor().name()
              << ") request failed: " << opencensus::common::ToString(status)
              << "\n";
    return false;
  }
  registered_descriptors_.emplace_hint(it, descriptor.name(), descriptor);
  return true;
}

}  // namespace

// static
void StackdriverExporter::Register(StackdriverOptions&& opts) {
  opencensus::stats::StatsExporter::RegisterPushHandler(
      absl::WrapUnique(new Handler(std::move(opts))));
}

// static, DEPRECATED
void StackdriverExporter::Register(StackdriverOptions& opts) {
  // Copy opts but take ownership of metric_service_stub.
  StackdriverOptions copied_opts;
  copied_opts.project_id = opts.project_id;
  copied_opts.opencensus_task = opts.opencensus_task;
  copied_opts.rpc_deadline = opts.rpc_deadline;
  copied_opts.monitored_resource = opts.monitored_resource;
  copied_opts.per_metric_monitored_resource =
      opts.per_metric_monitored_resource;
  copied_opts.metric_name_prefix = opts.metric_name_prefix;
  copied_opts.metric_service_stub = std::move(opts.metric_service_stub);
  opencensus::stats::StatsExporter::RegisterPushHandler(
      absl::WrapUnique(new Handler(std::move(opts))));
}

// static, DEPRECATED
void StackdriverExporter::Register(absl::string_view project_id,
                                   absl::string_view opencensus_task) {
  StackdriverOptions opts;
  opts.project_id = std::string(project_id);
  opts.opencensus_task = std::string(opencensus_task);
  Register(std::move(opts));
}

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
