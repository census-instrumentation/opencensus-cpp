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

workspace(name = "io_opencensus_cpp")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Build rules for C++ projects.
http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-master",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/master.zip"],
)

# We depend on Abseil.
http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
)

# GoogleTest framework.
# Only needed for tests, not to build the OpenCensus library.
http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-master",
    urls = ["https://github.com/google/googletest/archive/master.zip"],
)

# Google Benchmark library.
# Only needed for benchmarks, not to build the OpenCensus library.
http_archive(
    name = "com_github_google_benchmark",
    strip_prefix = "benchmark-master",
    urls = ["https://github.com/google/benchmark/archive/master.zip"],
)

# gRPC
http_archive(
    name = "com_github_grpc_grpc",
    strip_prefix = "grpc-master",
    urls = ["https://github.com/grpc/grpc/archive/master.tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# grpc_deps() cannot load() its deps, this WORKSPACE has to do it.
# See also: https://github.com/bazelbuild/bazel/issues/1943
load(
    "@build_bazel_rules_apple//apple:repositories.bzl",
    "apple_rules_dependencies",
)

apple_rules_dependencies()

load(
    "@build_bazel_apple_support//lib:repositories.bzl",
    "apple_support_dependencies",
)

apple_support_dependencies()

load("@upb//bazel:repository_defs.bzl", "bazel_version_repository")

bazel_version_repository(name = "bazel_version")

# Used by prometheus-cpp.
local_repository(
    name = "net_zlib_zlib",
    path = "tools/zlib",
)

# Prometheus client library - used by Prometheus exporter.
http_archive(
    name = "com_github_jupp0r_prometheus_cpp",
    strip_prefix = "prometheus-cpp-master",
    urls = ["https://github.com/jupp0r/prometheus-cpp/archive/master.zip"],
)

load("@com_github_jupp0r_prometheus_cpp//:repositories.bzl", "load_civetweb")

# Load Prometheus dependencies individually since we load some of them above.
load_civetweb()

# Curl library - used by zipkin exporter.
http_archive(
    name = "com_github_curl",
    build_file_content =
        """
load("@io_opencensus_cpp//opencensus:curl.bzl", "CURL_COPTS")
package(features = ['no_copts_tokenization'])

config_setting(
    name = "windows",
    values = {"cpu": "x64_windows"},
    visibility = [ "//visibility:private" ],
)

config_setting(
    name = "osx",
    values = {"cpu": "darwin"},
    visibility = [ "//visibility:private" ],
)

cc_library(
    name = "curl",
    srcs = glob([
        "lib/**/*.c",
    ]),
    hdrs = glob([
        "include/curl/*.h",
        "lib/**/*.h",
    ]),
    includes = ["include/", "lib/"],
    copts = CURL_COPTS + [
        "-DOS=\\"os\\"",
        "-DCURL_EXTERN_SYMBOL=__attribute__((__visibility__(\\"default\\")))",
    ],
    visibility = ["//visibility:public"],
)
""",
    strip_prefix = "curl-master",
    urls = ["https://github.com/curl/curl/archive/master.zip"],
)

# Rapidjson library - used by zipkin exporter.
http_archive(
    name = "com_github_tencent_rapidjson",
    build_file_content =
        """
cc_library(
    name = "rapidjson",
    srcs = [],
    hdrs = glob([
        "include/rapidjson/*.h",
        "include/rapidjson/internal/*.h",
        "include/rapidjson/error/*.h",
    ]),
    includes = ["include/"],
    visibility = ["//visibility:public"],
)
""",
    strip_prefix = "rapidjson-master",
    urls = ["https://github.com/Tencent/rapidjson/archive/master.zip"],
)

# Google APIs - used by Stackdriver exporter.
http_archive(
    name = "com_google_googleapis",
    strip_prefix = "googleapis-master",
    urls = ["https://github.com/googleapis/googleapis/archive/master.zip"],
)

load("@com_google_googleapis//:repository_rules.bzl", "switched_rules_by_language")

switched_rules_by_language(
    name = "com_google_googleapis_imports",
    cc = True,
    grpc = True,
)

# Needed by @opencensus_proto.
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "9fb16af4d4836c8222142e54c9efa0bb5fc562ffc893ce2abeac3e25daead144",
    urls = [
        "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/rules_go/releases/download/0.19.0/rules_go-0.19.0.tar.gz",
        "https://github.com/bazelbuild/rules_go/releases/download/0.19.0/rules_go-0.19.0.tar.gz",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

# Needed by @opencensus_proto.
http_archive(
    name = "grpc_java",
    strip_prefix = "grpc-java-master",
    urls = ["https://github.com/grpc/grpc-java/archive/master.zip"],
)

load("@grpc_java//:repositories.bzl", "grpc_java_repositories")

grpc_java_repositories(
    # Omit to avoid conflicts.
    omit_com_google_protobuf = True,
)

# OpenCensus protos - used by OcAgent exporter.
http_archive(
    name = "opencensus_proto",
    strip_prefix = "opencensus-proto-master/src",
    urls = ["https://github.com/census-instrumentation/opencensus-proto/archive/master.zip"],
)
