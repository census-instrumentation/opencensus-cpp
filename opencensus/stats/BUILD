# OpenCensus C++ Stats library.
#
# Copyright 2017, OpenCensus Authors
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

# The public stats API.
cc_library(
    name = "stats",
    hdrs = [
        "internal/aggregation_window.h",
        "internal/set_aggregation_window.h",
        "stats.h",
    ],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":core",
        ":recording",
    ],
)

# Test-only utilities.
cc_library(
    name = "test_utils",
    testonly = 1,
    srcs = ["testing/test_utils.cc"],
    hdrs = ["testing/test_utils.h"],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":core",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "core",
    srcs = [
        "internal/aggregation.cc",
        "internal/aggregation_window.cc",
        "internal/bucket_boundaries.cc",
        "internal/delta_producer.cc",
        "internal/distribution.cc",
        "internal/measure.cc",
        "internal/measure_data.cc",
        "internal/measure_descriptor.cc",
        "internal/measure_registry.cc",
        "internal/measure_registry_impl.cc",
        "internal/set_aggregation_window.cc",
        "internal/stats_exporter.cc",
        "internal/stats_manager.cc",
        "internal/tag_set.cc",
        "internal/view.cc",
        "internal/view_data.cc",
        "internal/view_data_impl.cc",
        "internal/view_descriptor.cc",
    ],
    hdrs = [
        "aggregation.h",
        "bucket_boundaries.h",
        "distribution.h",
        "internal/aggregation_window.h",
        "internal/delta_producer.h",
        "internal/measure_data.h",
        "internal/measure_registry_impl.h",
        "internal/set_aggregation_window.h",
        "internal/stats_exporter_impl.h",
        "internal/stats_manager.h",
        "internal/view_data_impl.h",
        "measure.h",
        "measure_descriptor.h",
        "measure_registry.h",
        "stats_exporter.h",
        "tag_set.h",
        "view.h",
        "view_data.h",
        "view_descriptor.h",
    ],
    copts = DEFAULT_COPTS,
    deps = [
        "//opencensus/common/internal:stats_object",
        "//opencensus/common/internal:string_vector_hash",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "recording",
    srcs = ["internal/recording.cc"],
    hdrs = ["recording.h"],
    copts = DEFAULT_COPTS,
    deps = [
        ":core",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

# Tests
# ========================================================================= #

cc_test(
    name = "debug_string_test",
    srcs = ["internal/debug_string_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "delta_producer_stats_test",
    srcs = ["internal/delta_producer_stats_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "distribution_test",
    srcs = ["internal/distribution_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        ":test_utils",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "bucket_boundaries_test",
    srcs = ["internal/bucket_boundaries_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "measure_data_test",
    size = "small",
    srcs = ["internal/measure_data_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        ":test_utils",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "measure_registry_test",
    srcs = ["internal/measure_registry_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stats_exporter_test",
    srcs = ["internal/stats_exporter_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        ":recording",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stats_manager_test",
    srcs = ["internal/stats_manager_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        ":recording",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "tag_set_test",
    size = "small",
    srcs = ["internal/tag_set_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "view_data_impl_test",
    srcs = ["internal/view_data_impl_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "view_data_test",
    srcs = ["internal/view_data_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":core",
        ":test_utils",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

# Benchmarks
# ========================================================================= #
cc_binary(
    name = "stats_manager_benchmark",
    testonly = 1,
    srcs = ["internal/stats_manager_benchmark.cc"],
    copts = TEST_COPTS,
    linkopts = ["-pthread"],  # Required for absl/synchronization bits.
    linkstatic = 1,
    deps = [
        ":core",
        ":recording",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@com_google_benchmark//:benchmark",
    ],
)
