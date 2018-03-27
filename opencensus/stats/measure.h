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

#ifndef OPENCENSUS_STATS_MEASURE_H_
#define OPENCENSUS_STATS_MEASURE_H_

#include "opencensus/stats/measure_descriptor.h"

#include <cstdint>

namespace opencensus {
namespace stats {

// A Measure represents a certain type of record, such as the latency of a
// request. Value events are recorded against measures, and a view specifying
// that measure can retrieve the data for those events. Measures can only be
// obtained from the static functions MeasureRegistry::Register() and
// MeasureRegistry::GetMeasureByName().
// Measure is immutable, and should be passed by value.
template <typename MeasureT>
class Measure final {
 public:
  // Retrieves a copy of the Measure's descriptor. This is expensive, requiring
  // a lookup in the MeasureRegistry.
  const MeasureDescriptor& GetDescriptor() const;

  // Returns true if the measure is valid and false otherwise. Recording with an
  // invalid Measure logs an error and assert-fails in debug mode.
  bool IsValid() const;

  Measure(const Measure<MeasureT>& other) : id_(other.id_) {}
  bool operator==(Measure<MeasureT> other) const { return id_ == other.id_; }

 private:
  friend class Measurement;
  friend class MeasureRegistryImpl;
  explicit Measure(uint64_t id);

  const uint64_t id_;
};

typedef Measure<double> MeasureDouble;
typedef Measure<int64_t> MeasureInt;

// Measurement is an immutable pair of a Measure and corresponding value to
// record--refer to comments in recording.h for further information.
// TODO: Write a non-compilation test.
// TODO: Make mismatching types produce a more informative error.
class Measurement final {
 public:
  template <typename T, typename std::enable_if<
                            std::is_floating_point<T>::value>::type* = nullptr>
  Measurement(MeasureDouble measure, T value)
      : id_(measure.id_), value_double_(value) {}
  template <typename T, typename std::enable_if<
                            std::is_integral<T>::value>::type* = nullptr>
  Measurement(MeasureInt measure, T value)
      : id_(measure.id_), value_int_(value) {}

 private:
  friend class StatsManager;
  friend class Delta;

  const uint64_t id_;
  union {
    const double value_double_;
    const int64_t value_int_;
  };
};

template <>
bool MeasureDouble::IsValid() const;
template <>
bool MeasureInt::IsValid() const;
extern template class Measure<double>;
extern template class Measure<int64_t>;

}  // namespace stats
}  // namespace opencensus

#endif  // OPENCENSUS_STATS_MEASURE_H_
