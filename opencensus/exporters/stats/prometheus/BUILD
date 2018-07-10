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

load("//opencensus:copts.bzl", "DEFAULT_COPTS", "TEST_COPTS")

licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:private"])

cc_library(
    name = "prometheus_exporter",
    srcs = ["internal/prometheus_exporter.cc"],
    hdrs = ["prometheus_exporter.h"],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":prometheus_utils",
        "//opencensus/stats",
        "@com_github_jupp0r_prometheus_cpp//core",
    ],
)

# Internal libraries.
# ========================================================================= #

cc_library(
    name = "prometheus_utils",
    srcs = ["internal/prometheus_utils.cc"],
    hdrs = ["internal/prometheus_utils.h"],
    copts = DEFAULT_COPTS,
    deps = [
        "//opencensus/stats",
        "@com_github_jupp0r_prometheus_cpp//core",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

# Tests.
# ========================================================================= #

cc_test(
    name = "prometheus_utils_test",
    srcs = ["internal/prometheus_utils_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":prometheus_utils",
        "//opencensus/stats",
        "//opencensus/stats:test_utils",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "prometheus_test_server",
    srcs = ["internal/prometheus_test_server.cc"],
    deps = [
        ":prometheus_exporter",
        "//opencensus/stats",
        "@com_github_jupp0r_prometheus_cpp//pull",
        "@com_google_absl//absl/time",
    ],
)
