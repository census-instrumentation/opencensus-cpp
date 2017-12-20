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

#include "opencensus/stats/measure_registry.h"

#include "opencensus/stats/internal/measure_registry_impl.h"

namespace opencensus {
namespace stats {

// static
MeasureDouble MeasureRegistry::RegisterDouble(absl::string_view name,
                                              absl::string_view units,
                                              absl::string_view description) {
  return MeasureRegistryImpl::Get()->RegisterDouble(name, units, description);
}

// static
MeasureInt MeasureRegistry::RegisterInt(absl::string_view name,
                                        absl::string_view units,
                                        absl::string_view description) {
  return MeasureRegistryImpl::Get()->RegisterInt(name, units, description);
}

// static
const MeasureDescriptor& MeasureRegistry::GetDescriptorByName(
    absl::string_view name) {
  return MeasureRegistryImpl::Get()->GetDescriptorByName(name);
}

// static
MeasureDouble MeasureRegistry::GetMeasureDoubleByName(absl::string_view name) {
  return MeasureRegistryImpl::Get()->GetMeasureDoubleByName(name);
}

// static
MeasureInt MeasureRegistry::GetMeasureIntByName(absl::string_view name) {
  return MeasureRegistryImpl::Get()->GetMeasureIntByName(name);
}

}  // namespace stats
}  // namespace opencensus
