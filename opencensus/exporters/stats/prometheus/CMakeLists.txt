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

opencensus_lib(exporters_stats_prometheus_exporter
               PUBLIC
               SRCS
               internal/prometheus_exporter.cc
               DEPS
               exporters_stats_prometheus_utils
               stats)

opencensus_lib(exporters_stats_prometheus_utils
               PUBLIC
               SRCS
               internal/prometheus_utils.cc
               DEPS
               stats
               absl::strings
               absl::time)
