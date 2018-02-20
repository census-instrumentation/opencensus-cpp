# End-to-end tests for stats exporter libraries.
#
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

licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:private"])

cc_test(
    name = "stackdriver_e2e_test",
    srcs = ["stackdriver_e2e_test.cc"],
    tags = [
        "manual",
    ],
    deps = [
        "//google/monitoring/v3:common",
        "//google/monitoring/v3:metric",
        "//google/monitoring/v3:metric_service",
        "//opencensus/exporters/stats:stackdriver_exporter",
        "//opencensus/exporters/stats:stackdriver_utils",
        "//opencensus/exporters/stats:test_utils",
        "//opencensus/stats",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)
