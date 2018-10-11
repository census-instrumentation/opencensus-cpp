# OpenCensus C++ Tags library.
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

package(default_visibility = ["//visibility:private"])

# The public tags API.
cc_library(
    name = "tags",
    srcs = [
        "internal/tag_key.cc",
        "internal/tag_map.cc",
    ],
    hdrs = [
        "tag_key.h",
        "tag_map.h",
    ],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//opencensus/common/internal:hash_mix",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
    ],
)

cc_library(
    name = "scoped_tags",
    srcs = ["internal/scoped_tags.cc"],
    hdrs = ["scoped_tags.h"],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":tags",
        "//opencensus/context",
    ],
)

cc_library(
    name = "with_tag_map",
    srcs = ["internal/with_tag_map.cc"],
    hdrs = ["with_tag_map.h"],
    copts = DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":tags",
        "//opencensus/context",
    ],
)

# Tests
# ========================================================================= #

cc_test(
    name = "tag_key_test",
    srcs = ["internal/tag_key_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":tags",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "tag_map_test",
    srcs = ["internal/tag_map_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":tags",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "scoped_tags_test",
    srcs = ["internal/scoped_tags_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":scoped_tags",
        ":tags",
        "//opencensus/context",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "with_tag_map_test",
    srcs = ["internal/with_tag_map_test.cc"],
    copts = TEST_COPTS,
    deps = [
        ":tags",
        ":with_tag_map",
        "//opencensus/context",
        "@com_google_googletest//:gtest_main",
    ],
)

# Benchmarks
# ========================================================================= #

cc_binary(
    name = "with_tag_map_benchmark",
    testonly = 1,
    srcs = ["internal/with_tag_map_benchmark.cc"],
    copts = TEST_COPTS,
    linkopts = ["-pthread"],  # Required for absl/synchronization bits.
    linkstatic = 1,
    deps = [
        ":tags",
        ":with_tag_map",
        "@com_github_google_benchmark//:benchmark",
    ],
)
