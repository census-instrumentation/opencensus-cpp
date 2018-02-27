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

#ifndef OPENCENSUS_STATS_MEASURE_REGISTRY_H_
#define OPENCENSUS_STATS_MEASURE_REGISTRY_H_

#include "opencensus/stats/measure.h"
#include "opencensus/stats/measure_descriptor.h"

namespace opencensus {
namespace stats {

// The MeasureRegistry keeps a record of all MeasureDescriptors registered,
// providing functions for registering measures and querying their metadata by
// name or handle.
// MeasureRegistry is thread-safe.
class MeasureRegistry final {
 public:
  // Registers a MeasureDescriptor, returning a Measure that can be used to
  // record values for or create views on that measure. Only one Measure may be
  // registered under a certain name; subsequent registrations will fail,
  // returning an invalid measure. Register* functions should only be called by
  // the owner of a measure--other users should use GetMeasureByName. If there
  // are multiple competing owners (e.g. for a generic resource such as "RPC
  // latency" shared between RPC libraries) check whether the measure is
  // registered with GetMeasure*ByName before registering it.
  //
  // 'name' should be a globally unique identifier. It is recommended that this
  //   be in the format "<domain>/<path>", e.g. "example.com/client/foo_usage".
  // 'units' are the units of recorded values. The recommended grammar is:
  //     - Expression = Component { "." Component } {"/" Component }
  //     - Component = [ PREFIX ] UNIT [ Annotation ] | Annotation | "1"
  //     - Annotation = "{" NAME "}"
  //   For example, string “MBy{transmitted}/ms” stands for megabytes per
  //   milliseconds, and the annotation transmitted inside {} is just a comment
  //   of the unit.
  // 'description' is a human-readable description of what the measure's
  //   values represent.
  static MeasureDouble RegisterDouble(absl::string_view name,
                                      absl::string_view units,
                                      absl::string_view description);
  static MeasureInt RegisterInt(absl::string_view name, absl::string_view units,
                                absl::string_view description);

  // Returns the descriptor of the measure registered under 'name' if one is
  // registered, and a descriptor with an empty name otherwise.
  static const MeasureDescriptor& GetDescriptorByName(absl::string_view name);

  // Returns a measure for the registered MeasureDescriptor with the
  // provided name, if one exists, and an invalid Measure otherwise.
  static MeasureDouble GetMeasureDoubleByName(absl::string_view name);
  static MeasureInt GetMeasureIntByName(absl::string_view name);
};

}  // namespace stats
}  // namespace opencensus

#endif  // OPENCENSUS_STATS_MEASURE_REGISTRY_H_
