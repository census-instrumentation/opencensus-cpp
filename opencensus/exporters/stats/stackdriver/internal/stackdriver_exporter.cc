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

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "google/monitoring/v3/metric_service.grpc.pb.h"
#include "google/protobuf/empty.pb.h"
#include "include/grpc++/grpc++.h"
#include "opencensus/exporters/stats/stackdriver/internal/stackdriver_utils.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

namespace {

constexpr char kGoogleStackdriverStatsAddress[] = "monitoring.googleapis.com";
constexpr char kProjectIdPrefix[] = "projects/";
constexpr int kTimeSeriesBatchSize = 3;

}  // namespace

class StackdriverExporter::Handler
    : public ::opencensus::stats::StatsExporter::Handler {
 public:
  Handler(absl::string_view project_id, absl::string_view opencensus_task);

  void ExportViewData(const opencensus::stats::ViewDescriptor& descriptor,
                      const opencensus::stats::ViewData& data)
      LOCKS_EXCLUDED(mu_) override;

 private:
  // Registers 'descriptor' with Stackdriver if no view by that name has been
  // registered by this, and adds it to registered_descriptors_ if successful.
  // Returns true if the view has already been registered or registration is
  // successful, and false if the registration fails or the name has already
  // been registered with different parameters.
  bool MaybeRegisterView(const opencensus::stats::ViewDescriptor& descriptor)
      EXCLUSIVE_LOCKS_REQUIRED(mu_);

  const std::string project_id_;
  const std::string opencensus_task_;
  const std::unique_ptr<google::monitoring::v3::MetricService::Stub> stub_;
  mutable absl::Mutex mu_;
  std::unordered_map<std::string, opencensus::stats::ViewDescriptor>
      registered_descriptors_ GUARDED_BY(mu_);
};

StackdriverExporter::Handler::Handler(absl::string_view project_id,
                                      absl::string_view opencensus_task)
    : project_id_(absl::StrCat(kProjectIdPrefix, project_id)),
      opencensus_task_(opencensus_task),
      stub_(google::monitoring::v3::MetricService::NewStub(
          ::grpc::CreateChannel(kGoogleStackdriverStatsAddress,
                                ::grpc::GoogleDefaultCredentials()))) {}

void StackdriverExporter::Handler::ExportViewData(
    const opencensus::stats::ViewDescriptor& descriptor,
    const opencensus::stats::ViewData& data) {
  absl::MutexLock l(&mu_);
  // Stackdriver does not support interval aggregation.
  if (descriptor.aggregation_window().type() !=
      opencensus::stats::AggregationWindow::Type::kCumulative) {
    return;
  }
  if (!MaybeRegisterView(descriptor)) {
    return;
  }

  // TODO: use asynchronous RPCs.
  const auto time_series = MakeTimeSeries(descriptor, data, opencensus_task_);
  int i = 0;
  while (i < time_series.size()) {
    auto request = google::monitoring::v3::CreateTimeSeriesRequest();
    request.set_name(project_id_);
    // TODO: refactor to avoid copying each timeseries.
    for (int batch_index = 0; batch_index < kTimeSeriesBatchSize;
         ++batch_index) {
      *request.add_time_series() = time_series[i];
      if (++i >= time_series.size()) {
        break;
      }
    }
    ::grpc::ClientContext context;
    google::protobuf::Empty response;
    ::grpc::Status status =
        stub_->CreateTimeSeries(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "CreateTimeSeries request failed: " << status.error_details()
                << "\n";
    }
  }
}

bool StackdriverExporter::Handler::MaybeRegisterView(
    const opencensus::stats::ViewDescriptor& descriptor) {
  const auto& it = registered_descriptors_.find(descriptor.name());
  if (it != registered_descriptors_.end()) {
    return it->second == descriptor;
  }

  auto request = google::monitoring::v3::CreateMetricDescriptorRequest();
  request.set_name(project_id_);
  SetMetricDescriptor(project_id_, descriptor,
                      request.mutable_metric_descriptor());
  ::grpc::ClientContext context;
  google::api::MetricDescriptor response;
  ::grpc::Status status =
      stub_->CreateMetricDescriptor(&context, request, &response);
  if (!status.ok()) {
    std::cerr << "CreateMetricDescriptor request failed: "
              << status.error_details() << "\n";
    return false;
  }
  registered_descriptors_.emplace_hint(it, descriptor.name(), descriptor);
  return true;
}

// static
void StackdriverExporter::Register(absl::string_view project_id,
                                   absl::string_view opencensus_task) {
  opencensus::stats::StatsExporter::RegisterHandler(absl::WrapUnique(
      new StackdriverExporter::Handler(project_id, opencensus_task)));
}

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
