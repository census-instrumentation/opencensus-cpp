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

#include "opencensus/stats/internal/measure_data.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

#include "absl/base/macros.h"
#include "absl/types/span.h"
#include "opencensus/stats/bucket_boundaries.h"
#include "opencensus/stats/distribution.h"

namespace opencensus {
namespace stats {

MeasureData::MeasureData(absl::Span<const BucketBoundaries> boundaries)
    : boundaries_(boundaries) {
  histograms_.reserve(boundaries_.size());
  for (const auto& b : boundaries_) {
    histograms_.emplace_back(b.num_buckets());
  }
}

void MeasureData::Add(double value) {
  // Update using the method of provisional means.
  ++count_;
  ABSL_ASSERT(count_ > 0 && "Histogram count overflow.");
  const double old_mean = mean_;
  mean_ += (value - mean_) / count_;
  sum_of_squared_deviation_ =
      sum_of_squared_deviation_ + (value - old_mean) * (value - mean_);

  min_ = std::min(value, min_);
  max_ = std::max(value, max_);

  for (int i = 0; i < boundaries_.size(); ++i) {
    ++histograms_[i][boundaries_[i].BucketForValue(value)];
  }
}

void MeasureData::AddToDistribution(Distribution* distribution) const {
  // This uses the method of provisional means generalized for multiple values
  // in both datasets.
  const double new_count = distribution->count_ + count_;
  const double new_mean =
      distribution->mean_ + (mean_ - distribution->mean_) * count_ / new_count;
  distribution->sum_of_squared_deviation_ +=
      sum_of_squared_deviation_ +
      distribution->count_ * std::pow(distribution->mean_, 2) +
      count_ * std::pow(mean_, 2) - new_count * std::pow(new_mean, 2);
  distribution->count_ = new_count;
  distribution->mean_ = new_mean;

  distribution->min_ = std::min(distribution->min_, min_);
  distribution->max_ = std::max(distribution->max_, max_);

  int histogram_index = std::find(boundaries_.begin(), boundaries_.end(),
                                  distribution->bucket_boundaries()) -
                        boundaries_.begin();
  if (histogram_index >= histograms_.size()) {
    std::cerr << "No matching BucketBoundaries in AddToDistribution\n";
    ABSL_ASSERT(false);
    // Add to the underflow bucket, to avoid downstream errors from the sum of
    // bucket counts not matching the total count.
    distribution->bucket_counts_[0] += count_;
  } else {
    for (int i = 0; i < histograms_[histogram_index].size(); ++i) {
      distribution->bucket_counts_[i] += histograms_[histogram_index][i];
    }
  }
}

}  // namespace stats
}  // namespace opencensus
