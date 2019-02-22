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
    # TODO: Unpin grpc and go back to master when
    # https://github.com/grpc/grpc/pull/18082 is resolved.
    # (build is broken on darwin)
    strip_prefix = "grpc-b03e014ad82c39b7c91d794bc775c0d4ec187e1b",
    urls = ["https://github.com/grpc/grpc/archive/b03e014ad82c39b7c91d794bc775c0d4ec187e1b.tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

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
    defines = ["RAPIDJSON_HAS_STDSTRING=1",],
    visibility = ["//visibility:public"],
)
""",
    strip_prefix = "rapidjson-master",
    urls = ["https://github.com/Tencent/rapidjson/archive/master.zip"],
)
