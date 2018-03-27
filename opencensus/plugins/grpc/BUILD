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

# Libraries
# ========================================================================= #

cc_library(
    name = "grpc_plugin",
    srcs = [
        "internal/channel_filter.cc",
        "internal/client_filter.cc",
        "internal/filter.cc",
        "internal/grpc_plugin.cc",
        "internal/measures.cc",
        "internal/server_filter.cc",
        "internal/views.cc",
    ],
    hdrs = [
        "grpc_plugin.h",
        "internal/channel_filter.h",
        "internal/client_filter.h",
        "internal/filter.h",
        "internal/measures.h",
        "internal/rpc_encoding.h",
        "internal/server_filter.h",
    ],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//opencensus/stats",
        "//opencensus/trace",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base:endian",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

# Tests
# ========================================================================= #

# TODO: grpc_plugin_end2end_test

cc_test(
    name = "stats_plugin_end2end_test",
    srcs = ["internal/stats_plugin_end2end_test.cc"],
    copts = TEST_COPTS,
    flaky = True,  # RPCs are unreliable.
    linkopts = [
        "-pthread",
    ],
    deps = [
        ":grpc_plugin",
        "//opencensus/plugins/grpc/internal/testing:echo_proto",
        "//opencensus/stats",
        "//opencensus/stats:test_utils",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

# Benchmarks
# ========================================================================= #
cc_binary(
    name = "grpc_plugin_benchmark",
    testonly = 1,
    srcs = ["internal/grpc_plugin_benchmark.cc"],
    copts = TEST_COPTS,
    linkopts = [
        "-pthread",
    ],
    linkstatic = 1,
    deps = [
        ":grpc_plugin",
        "//opencensus/plugins/grpc/internal/testing:echo_proto",
        "//opencensus/stats",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/strings",
        "@com_google_benchmark//:benchmark",
    ],
)
