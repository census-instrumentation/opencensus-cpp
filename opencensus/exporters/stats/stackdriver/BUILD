# OpenCensus C++ exporter libraries.
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

load("//opencensus:copts.bzl", "DEFAULT_COPTS", "TEST_COPTS")

licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//opencensus/exporters/stats:__subpackages__"])

# Public exporter libraries.

cc_library(
    name = "stackdriver_exporter",
    srcs = ["internal/stackdriver_exporter.cc"],
    hdrs = ["stackdriver_exporter.h"],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":stackdriver_utils",
        "//google/monitoring/v3:metric_service",
        "//opencensus/stats",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "stdout_exporter",
    srcs = ["internal/stdout_exporter.cc"],
    hdrs = ["stdout_exporter.h"],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//opencensus/stats",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

# Internal libraries.
# ========================================================================= #

cc_library(
    name = "stackdriver_utils",
    srcs = ["internal/stackdriver_utils.cc"],
    hdrs = ["internal/stackdriver_utils.h"],
    copts = DEFAULT_COPTS,
    deps = [
        "//google/api:distribution",
        "//google/api:label",
        "//google/api:metric",
        "//google/api:monitored_resource",
        "//google/monitoring/v3:common",
        "//google/monitoring/v3:metric",
        "//opencensus/stats",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "test_utils",
    testonly = 1,
    srcs = ["internal/testing/time_series_matcher.cc"],
    hdrs = ["internal/testing/time_series_matcher.h"],
    deps = [
        "//google/api:distribution",
        "//google/api:metric",
        "//google/monitoring/v3:common",
        "//google/monitoring/v3:metric",
        "//opencensus/stats",
        "@com_google_googletest//:gtest",
    ],
)

# Tests.
# ========================================================================= #

cc_test(
    name = "stackdriver_utils_test",
    srcs = ["internal/stackdriver_utils_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":stackdriver_utils",
        ":test_utils",
        "//google/api:distribution",
        "//google/api:label",
        "//google/api:metric",
        "//google/monitoring/v3:common",
        "//google/monitoring/v3:metric",
        "//opencensus/stats",
        "//opencensus/stats:test_utils",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
