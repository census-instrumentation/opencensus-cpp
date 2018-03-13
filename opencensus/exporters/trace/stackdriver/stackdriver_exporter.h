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

#ifndef OPENCENSUS_EXPORTERS_TRACE_STACKDRIVER_STACKDRIVER_EXPORTER_H_
#define OPENCENSUS_EXPORTERS_TRACE_STACKDRIVER_STACKDRIVER_EXPORTER_H_

#include "absl/strings/string_view.h"

namespace opencensus {
namespace exporters {
namespace trace {

class StackdriverExporter {
 public:
  // Registers the exporter and sets the project ID.
  static void Register(absl::string_view project_id);

 private:
  StackdriverExporter() = delete;
};

}  // namespace trace
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_TRACE_STACKDRIVER_STACKDRIVER_EXPORTER_H_
