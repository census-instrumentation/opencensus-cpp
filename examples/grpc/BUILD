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
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:private"])

proto_library(
    name = "hello_proto",
    srcs = ["hello.proto"],
)

cc_proto_library(
    name = "hello_cc_proto",
    deps = [":hello_proto"],
)

cc_grpc_library(
    name = "hello_cc_grpc",
    srcs = [":hello_proto"],
    grpc_only = True,
    deps = [":hello_cc_proto"],
)

cc_library(
    name = "exporters",
    srcs = ["exporters.cc"],
    hdrs = ["exporters.h"],
    copts = DEFAULT_COPTS,
    deps = [
        "//opencensus/exporters/stats/stackdriver:stackdriver_exporter",
        "//opencensus/exporters/stats/stdout:stdout_exporter",
        "//opencensus/exporters/trace/ocagent:ocagent_exporter",
        "//opencensus/exporters/trace/stackdriver:stackdriver_exporter",
        "//opencensus/exporters/trace/stdout:stdout_exporter",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "hello_client",
    srcs = ["hello_client.cc"],
    copts = DEFAULT_COPTS,
    deps = [
        ":exporters",
        ":hello_cc_grpc",
        ":hello_cc_proto",
        "//opencensus/tags",
        "//opencensus/tags:context_util",
        "//opencensus/tags:with_tag_map",
        "//opencensus/trace",
        "//opencensus/trace:context_util",
        "//opencensus/trace:with_span",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc_opencensus_plugin",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
    ],
)

cc_binary(
    name = "hello_server",
    srcs = ["hello_server.cc"],
    copts = DEFAULT_COPTS,
    deps = [
        ":exporters",
        ":hello_cc_grpc",
        ":hello_cc_proto",
        "//opencensus/exporters/stats/prometheus:prometheus_exporter",
        "//opencensus/tags",
        "//opencensus/tags:context_util",
        "//opencensus/trace",
        "//opencensus/trace:context_util",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc_opencensus_plugin",
        "@com_github_jupp0r_prometheus_cpp//pull",
        "@com_google_absl//absl/strings",
    ],
)
