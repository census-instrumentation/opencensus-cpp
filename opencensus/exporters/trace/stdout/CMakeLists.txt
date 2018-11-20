# ~~~
# Copyright 2018, OpenCensus Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

add_library(exporters.trace.stdout_exporter internal/stdout_exporter.cc)
add_library(${PROJECT_NAME}::exporters.trace.stdout_exporter ALIAS
            exporters.trace.stdout_exporter)

target_link_libraries(exporters.trace.stdout_exporter
                      PRIVATE trace absl::base absl::memory)

# ----------------------------------------------------------------------

opencensus_test(exporters.trace.stdout_exporter_test
                internal/stdout_exporter_test.cc
                exporters.trace.stdout_exporter
                trace
                absl::time
                gmock)
