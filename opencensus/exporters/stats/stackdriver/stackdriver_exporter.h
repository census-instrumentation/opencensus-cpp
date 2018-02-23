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

#ifndef OPENCENSUS_EXPORTERS_STATS_STACKDRIVER_EXPORTER_H_
#define OPENCENSUS_EXPORTERS_STATS_STACKDRIVER_EXPORTER_H_

#include "absl/strings/string_view.h"
#include "opencensus/stats/stats.h"

namespace opencensus {
namespace exporters {
namespace stats {

// Exports stats for registered views (see opencensus/stats/stats_exporter.h) to
// Stackdriver.
// StackdriverExporter is thread-safe.
class StackdriverExporter : public ::opencensus::stats::StatsExporter::Handler {
 public:
  // Registers the exporter and sets the project ID and task value. The
  // following are required to communicate with stackdriver: a valid project ID,
  // a valid authentication key which has been generated for that project (with
  // the "Monitoring Editor" role), and the server security credentials. An
  // authentication key can be generated for your project in the gcp/stackdriver
  // settings for your account. Grpc will look for the key using the path
  // defined by GOOGLE_APPLICATION_CREDENTIALS environment variable (export
  // GOOGLE_APPLICATION_CREDENTIALS=<path_to_key>). The server security
  // credentials located in <grpc_path>/etc/roots.pem needs to be copied to
  // /usr/share/grpc/roots.pem
  // project_id should be the exact id of the project, as in the GCP console,
  // with no prefix--e.g. "sample-project-id". opencensus_task is used to
  // uniquely identify the task in Stackdriver. The recommended format is
  // "{LANGUAGE}-{PID}@{HOSTNAME}"; if {PID} is not available a random number
  // may be used.
  static void Register(absl::string_view project_id,
                       absl::string_view opencensus_task);

 private:
  class Handler;
};

}  // namespace stats
}  // namespace exporters
}  // namespace opencensus

#endif  // OPENCENSUS_EXPORTERS_STATS_STACKDRIVER_EXPORTER_H_
