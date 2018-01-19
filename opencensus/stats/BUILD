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

licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:private"])

# The public stats API.
cc_library(
    name = "stats",
    hdrs = ["stats.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":core",
        ":export",
        ":recording",
    ],
)

# Test-only utilities.
cc_library(
    name = "test_utils",
    testonly = 1,
    srcs = ["testing/test_utils.cc"],
    hdrs = ["testing/test_utils.h"],
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
        "internal/distribution.cc",
        "internal/measure.cc",
        "internal/measure_descriptor.cc",
        "internal/measure_registry.cc",
        "internal/measure_registry_impl.cc",
        "internal/stats_manager.cc",
        "internal/view_data.cc",
        "internal/view_data_impl.cc",
        "internal/view_descriptor.cc",
    ],
    hdrs = [
        "aggregation.h",
        "aggregation_window.h",
        "bucket_boundaries.h",
        "distribution.h",
        "internal/measure_registry_impl.h",
        "internal/stats_manager.h",
        "internal/view_data_impl.h",
        "measure.h",
        "measure_descriptor.h",
        "measure_registry.h",
        "view_data.h",
        "view_descriptor.h",
    ],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
        "//opencensus/common/internal:stats_object",
        "//opencensus/common/internal:string_vector_hash",
    ],
)

cc_library(
    name = "recording",
    srcs = ["internal/recording.cc"],
    hdrs = ["recording.h"],
    deps = [
        ":core",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "export",
    srcs = [
        "internal/stats_exporter.cc",
        "internal/view.cc",
    ],
    hdrs = [
        "stats_exporter.h",
        "view.h",
    ],
    deps = [
        ":core",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
    ],
)

# Tests
# ========================================================================= #

cc_test(
    name = "debug_string_test",
    srcs = ["internal/debug_string_test.cc"],
    deps = [
        ":core",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "distribution_test",
    srcs = ["internal/distribution_test.cc"],
    deps = [
        ":core",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "bucket_boundaries_test",
    srcs = ["internal/bucket_boundaries_test.cc"],
    deps = [
        ":core",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "measure_registry_test",
    srcs = ["internal/measure_registry_test.cc"],
    deps = [
        ":core",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stats_exporter_test",
    srcs = ["internal/stats_exporter_test.cc"],
    deps = [
        ":core",
        ":export",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stats_manager_test",
    srcs = ["internal/stats_manager_test.cc"],
    deps = [
        ":core",
        ":export",
        ":recording",
        "@com_google_absl//absl/types:optional",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "view_data_impl_test",
    srcs = ["internal/view_data_impl_test.cc"],
    deps = [
        ":core",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)
