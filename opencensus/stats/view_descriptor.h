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

#ifndef OPENCENSUS_STATS_VIEW_DESCRIPTOR_H_
#define OPENCENSUS_STATS_VIEW_DESCRIPTOR_H_

#include <cstdlib>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "opencensus/stats/aggregation.h"
#include "opencensus/stats/aggregation_window.h"
#include "opencensus/stats/measure_descriptor.h"

namespace opencensus {
namespace stats {

// ViewDescriptor provides metadata for a view, including its identity and the
// data to be collected.
// ViewDescriptor is a value type, and is thread-compatible.
// TODO: DOCS: Document members.
class ViewDescriptor final {
 public:
  ViewDescriptor();

  ViewDescriptor& set_name(absl::string_view name);
  const std::string& name() const { return name_; }

  // Sets the measure. If no measure is registered under 'name' any View created
  // with the descriptor will be invalid.
  ViewDescriptor& set_measure(absl::string_view name);

  const MeasureDescriptor& measure_descriptor() const;

  ViewDescriptor& set_aggregation(const Aggregation& aggregation);
  const Aggregation& aggregation() const { return aggregation_; }

  ViewDescriptor& set_aggregation_window(const AggregationWindow& window);
  const AggregationWindow& aggregation_window() const {
    return aggregation_window_;
  }

  ViewDescriptor& add_column(absl::string_view tag_key);
  size_t num_columns() const { return columns_.size(); }
  const std::vector<std::string>& columns() const { return columns_; }

  ViewDescriptor& set_description(absl::string_view description);
  const std::string& description() const { return description_; }

  std::string DebugString() const;

  bool operator==(const ViewDescriptor& other) const;
  bool operator!=(const ViewDescriptor& other) const {
    return !(*this == other);
  }

 private:
  friend class StatsManager;

  std::string name_;
  std::string measure_name_;
  uint64_t measure_id_;
  Aggregation aggregation_;
  AggregationWindow aggregation_window_;
  std::vector<std::string> columns_;
  std::string description_;
};

}  // namespace stats
}  // namespace opencensus

#endif  // OPENCENSUS_STATS_VIEW_DESCRIPTOR_H_
