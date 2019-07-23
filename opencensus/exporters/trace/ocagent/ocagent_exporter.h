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

#ifndef OPENCENSUS_EXPORTERS_TRACE_OCAGENT_OCAGENT_EXPORTER_H_
#define OPENCENSUS_EXPORTERS_TRACE_OCAGENT_OCAGENT_EXPORTER_H_

#include <string>

#include "absl/base/macros.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"

namespace opencensus {
namespace exporters {
namespace trace {

struct OcagentOptions {
  // The Ocagent address to use.
  std::string address;

  // The RPC deadline to use when exporting to Ocagent.
  absl::Duration rpc_deadline = absl::Seconds(5);
};

class OcagentExporter {
public:
  // Registers the exporter.
  static void Register(const OcagentOptions &opts);

  // TODO: Retire this:
  ABSL_DEPRECATED("Register() without OcagentOptions is deprecated and "
                  "will be removed on or after 2019-03-20")
  static void Register(absl::string_view address);

private:
  OcagentExporter() = delete;
};

} // namespace trace
} // namespace exporters
} // namespace opencensus

#endif // OPENCENSUS_EXPORTERS_TRACE_OCAGENT_OCAGENT_EXPORTER_H_
