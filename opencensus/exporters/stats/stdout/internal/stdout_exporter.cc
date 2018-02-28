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

#include "opencensus/exporters/stats/stdout/stdout_exporter.h"

#include <cstdint>
#include <functional>
#include <iostream>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

namespace {

// Functions to print data for different aggregation types.
std::string DataToString(double data) { return absl::StrCat(": ", data); }
std::string DataToString(int64_t data) { return absl::StrCat(": ", data); }
std::string DataToString(const opencensus::stats::Distribution& data) {
  return absl::StrCat("\n", data.DebugString());
}

}  // namespace

class StdoutExporter::Handler
    : public opencensus::stats::StatsExporter::Handler {
 public:
  void ExportViewData(const opencensus::stats::ViewDescriptor& descriptor,
                      const opencensus::stats::ViewData& data) override;

 private:
  // Implements ExportViewData for supported data types.
  template <typename DataValueT>
  void ExportViewDataImpl(
      const opencensus::stats::ViewDescriptor& descriptor,
      absl::Time start_time, absl::Time end_time,
      const opencensus::stats::ViewData::DataMap<DataValueT>& data);
};

// static
void StdoutExporter::Register() {
  opencensus::stats::StatsExporter::RegisterPushHandler(
      absl::make_unique<StdoutExporter::Handler>());
}

void StdoutExporter::Handler::ExportViewData(
    const opencensus::stats::ViewDescriptor& descriptor,
    const opencensus::stats::ViewData& data) {
  switch (data.type()) {
    case opencensus::stats::ViewData::Type::kDouble:
      ExportViewDataImpl(descriptor, data.start_time(), data.end_time(),
                         data.double_data());
      break;
    case opencensus::stats::ViewData::Type::kInt64:
      ExportViewDataImpl(descriptor, data.start_time(), data.end_time(),
                         data.int_data());
      break;
    case opencensus::stats::ViewData::Type::kDistribution:
      ExportViewDataImpl(descriptor, data.start_time(), data.end_time(),
                         data.distribution_data());
      break;
  }
}

template <typename DataValueT>
void StdoutExporter::Handler::ExportViewDataImpl(
    const opencensus::stats::ViewDescriptor& descriptor, absl::Time start_time,
    absl::Time end_time,
    const opencensus::stats::ViewData::DataMap<DataValueT>& data) {
  std::string output;
  absl::StrAppend(&output, "\nData for view \"", descriptor.name(), "\" from ",
                  absl::FormatTime(start_time), " to ",
                  absl::FormatTime(end_time), ":\n");
  for (const auto& tag_key : descriptor.columns()) {
    absl::StrAppend(
        &output, tag_key,
        tag_key.size() > 10 ? "" : std::string(10 - tag_key.size(), ' '));
  }
  absl::StrAppend(&output, "\n");
  for (const auto& row : data) {
    for (const auto& tag_value : row.first) {
      absl::StrAppend(
          &output, tag_value,
          tag_value.size() > 10 ? "" : std::string(10 - tag_value.size(), ' '));
    }
    absl::StrAppend(&output, DataToString(row.second), "\n");
  }
  absl::StrAppend(&output, "\n");
  std::cout << output;
}

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus
