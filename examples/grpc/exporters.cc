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

#include "examples/grpc/exporters.h"

#include <cstdlib>
#include <iostream>

#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/stats/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/ocagent/ocagent_exporter.h"
#include "opencensus/exporters/trace/stackdriver/stackdriver_exporter.h"
#include "opencensus/exporters/trace/stdout/stdout_exporter.h"

void RegisterExporters() {
  // For debugging, register exporters that just write to stdout.
  opencensus::exporters::stats::StdoutExporter::Register();
  opencensus::exporters::trace::StdoutExporter::Register();

  const char* project_id = getenv("STACKDRIVER_PROJECT_ID");
  if (project_id == nullptr) {
    std::cerr << "The STACKDRIVER_PROJECT_ID environment variable is not set: "
                 "not exporting to Stackdriver.\n";
  } else {
    std::cout << "RegisterStackdriverExporters:\n";
    std::cout << "  project_id = \"" << project_id << "\"\n";

    opencensus::exporters::stats::StackdriverOptions stats_opts;
    stats_opts.project_id = project_id;
    opencensus::exporters::stats::StackdriverExporter::Register(
        std::move(stats_opts));

    opencensus::exporters::trace::StackdriverOptions trace_opts;
    trace_opts.project_id = project_id;
    opencensus::exporters::trace::StackdriverExporter::Register(
        std::move(trace_opts));
  }

  const char* ocagent_address = getenv("OCAGENT_ADDRESS");
  if (ocagent_address == nullptr) {
    std::cerr << "The OCAGENT_ADDRESS environment variable is not set: not "
                 "exporting to OpenCensus Agent. (e.g. localhost:55678)\n";
  } else {
    opencensus::exporters::trace::OcAgentOptions opts;
    opts.address = ocagent_address;
    opencensus::exporters::trace::OcAgentExporter::Register(std::move(opts));
  }
}
