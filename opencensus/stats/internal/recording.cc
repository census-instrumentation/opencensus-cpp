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

#include "opencensus/stats/recording.h"

#include "absl/time/clock.h"
#include "opencensus/stats/internal/stats_manager.h"
#include "opencensus/stats/measure.h"

namespace opencensus {
namespace stats {

void Record(
    std::initializer_list<Measurement> measurements,
    std::initializer_list<std::pair<absl::string_view, absl::string_view>>
        tags) {
  StatsManager::Get()->Record(measurements, tags, absl::Now());
}

}  // namespace stats
}  // namespace opencensus
