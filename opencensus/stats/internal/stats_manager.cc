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

#include "opencensus/stats/internal/stats_manager.h"

#include <iostream>

#include "absl/base/macros.h"
#include "absl/time/time.h"

namespace opencensus {
namespace stats {

// TODO: See if it is possible to replace AssertHeld() with function
// annotations.
// TODO: Optimize selecting/sorting tag values for each view.

// ========================================================================== //
// StatsManager::ViewInformation

StatsManager::ViewInformation::ViewInformation(const ViewDescriptor& descriptor,
                                               absl::Mutex* mu)
    : descriptor_(descriptor), mu_(mu), data_(absl::Now(), descriptor) {}

bool StatsManager::ViewInformation::Matches(
    const ViewDescriptor& descriptor) const {
  return descriptor.aggregation() == descriptor_.aggregation() &&
         descriptor.aggregation_window_ == descriptor_.aggregation_window_ &&
         descriptor.columns() == descriptor_.columns();
}

int StatsManager::ViewInformation::num_consumers() const {
  mu_->AssertReaderHeld();
  return num_consumers_;
}

void StatsManager::ViewInformation::AddConsumer() {
  mu_->AssertHeld();
  ++num_consumers_;
}

int StatsManager::ViewInformation::RemoveConsumer() {
  mu_->AssertHeld();
  return --num_consumers_;
}

void StatsManager::ViewInformation::Record(
    double value,
    absl::Span<const std::pair<absl::string_view, absl::string_view>> tags,
    absl::Time now) {
  mu_->AssertHeld();
  std::vector<std::string> tag_values(descriptor_.columns().size());
  for (int i = 0; i < tag_values.size(); ++i) {
    const std::string& column = descriptor_.columns()[i];
    for (const auto& tag : tags) {
      if (tag.first == column) {
        tag_values[i] = std::string(tag.second);
        break;
      }
    }
  }
  data_.Add(value, tag_values, now);
}

ViewDataImpl StatsManager::ViewInformation::GetData() const {
  absl::ReaderMutexLock l(mu_);
  if (data_.type() == ViewDataImpl::Type::kStatsObject) {
    return ViewDataImpl(data_, absl::Now());
  } else {
    return data_;
  }
}

// ==========================================================================
// // StatsManager::MeasureInformation

void StatsManager::MeasureInformation::Record(
    double value,
    absl::Span<const std::pair<absl::string_view, absl::string_view>> tags,
    absl::Time now) {
  mu_->AssertHeld();
  for (auto& view : views_) {
    view->Record(value, tags, now);
  }
}

StatsManager::ViewInformation* StatsManager::MeasureInformation::AddConsumer(
    const ViewDescriptor& descriptor) {
  mu_->AssertHeld();
  for (auto& view : views_) {
    if (view->Matches(descriptor)) {
      view->AddConsumer();
      return view.get();
    }
  }
  views_.emplace_back(new ViewInformation(descriptor, mu_));
  return views_.back().get();
}

void StatsManager::MeasureInformation::RemoveView(
    const ViewInformation* handle) {
  mu_->AssertHeld();
  for (auto it = views_.begin(); it != views_.end(); ++it) {
    if (it->get() == handle) {
      ABSL_ASSERT((*it)->num_consumers() == 0);
      views_.erase(it);
      return;
    }
  }

  std::cerr << "Removing view from wrong measure.\n";
  ABSL_ASSERT(0);
}

// ==========================================================================
// // StatsManager

// static
StatsManager* StatsManager::Get() {
  static StatsManager* global_stats_manager = new StatsManager();
  return global_stats_manager;
}

void StatsManager::Record(
    std::initializer_list<Measurement> measurements,
    std::initializer_list<std::pair<absl::string_view, absl::string_view>> tags,
    absl::Time now) {
  absl::MutexLock l(&mu_);
  for (const auto& measurement : measurements) {
    if (MeasureRegistryImpl::IdValid(measurement.id_)) {
      const uint64_t index = MeasureRegistryImpl::IdToIndex(measurement.id_);
      switch (MeasureRegistryImpl::IdToType(measurement.id_)) {
        case MeasureDescriptor::Type::kDouble:
          measures_[index].Record(measurement.value_double_, tags, now);
          break;
        case MeasureDescriptor::Type::kInt64:
          measures_[index].Record(measurement.value_int_, tags, now);
          break;
      }
    }
  }
}

template <typename MeasureT>
void StatsManager::AddMeasure(Measure<MeasureT> measure) {
  absl::MutexLock l(&mu_);
  measures_.emplace_back(MeasureInformation(&mu_));
  ABSL_ASSERT(measures_.size() ==
              MeasureRegistryImpl::MeasureToIndex(measure) + 1);
}

template void StatsManager::AddMeasure(MeasureDouble measure);
template void StatsManager::AddMeasure(MeasureInt measure);

StatsManager::ViewInformation* StatsManager::AddConsumer(
    const ViewDescriptor& descriptor) {
  absl::MutexLock l(&mu_);
  if (!MeasureRegistryImpl::IdValid(descriptor.measure_id_)) {
    std::cerr
        << "Attempting to register a ViewDescriptor with an invalid measure:\n"
        << descriptor.DebugString() << "\n";
    return nullptr;
  }
  const uint64_t index = MeasureRegistryImpl::IdToIndex(descriptor.measure_id_);
  return measures_[index].AddConsumer(descriptor);
}

void StatsManager::RemoveConsumer(ViewInformation* handle) {
  absl::MutexLock l(&mu_);
  const int num_consumers_remaining = handle->RemoveConsumer();
  ABSL_ASSERT(num_consumers_remaining >= 0);
  if (num_consumers_remaining == 0) {
    const auto& descriptor = handle->view_descriptor();
    const uint64_t index =
        MeasureRegistryImpl::IdToIndex(descriptor.measure_id_);
    measures_[index].RemoveView(handle);
  }
}

}  // namespace stats
}  // namespace opencensus
