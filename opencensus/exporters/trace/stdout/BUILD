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

package(default_visibility = ["//visibility:public"])

# Libraries
# ========================================================================= #

cc_library(
    name = "stdout_exporter",
    srcs = ["stdout_exporter.cc"],
    hdrs = ["stdout_exporter.h"],
    copts = DEFAULT_COPTS,
    deps = [
        "//opencensus/trace",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "stackdriver_exporter",
    srcs = ["stackdriver_exporter.cc"],
    hdrs = ["stackdriver_exporter.h"],
    copts = DEFAULT_COPTS,
    deps = [
        "//google/devtools/cloudtrace/v2:tracing_proto",
        "//opencensus/trace",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)

# Tests
# ========================================================================= #

cc_test(
    name = "stdout_exporter_test",
    srcs = ["stdout_exporter_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":stdout_exporter",
        "//opencensus/trace",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stackdriver_exporter_test",
    srcs = ["stackdriver_exporter_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":stackdriver_exporter",
        "//opencensus/trace",
        "@com_github_grpc_grpc//test/core/util:gpr_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)
