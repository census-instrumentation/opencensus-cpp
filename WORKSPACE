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
    sha256 = "41407f630af76558a6da99f46a573eca108399aaff8905dfe7468cf0e904310c",
    strip_prefix = "abseil-cpp-540e2537b92cd4abfae6ceddfe24304345461f32",
    urls = ["https://github.com/abseil/abseil-cpp/archive/540e2537b92cd4abfae6ceddfe24304345461f32.zip"],
)

# GoogleTest framework.
# Only needed for tests, not to build the OpenCensus library.
http_archive(
    name = "com_google_googletest",
    sha256 = "72b93443943961eac7d28bbe6b303cb11cc7be63678e617c9bdeb6603f89161f",
    strip_prefix = "googletest-e04254989d262ad453ebd4f5cf07474014d81d52",
    urls = ["https://github.com/google/googletest/archive/e04254989d262ad453ebd4f5cf07474014d81d52.zip"],
)

# Google Benchmark library.
# Only needed for benchmarks, not to build the OpenCensus library.
http_archive(
    name = "com_github_google_benchmark",
    sha256 = "dadf44f5c4a4714a169ddb54e80d82cdc8bbe05e4b1ee4839dc645ed03ebb6bc",
    strip_prefix = "benchmark-785e2c3158589e8ef48c59ba80e48d76bdbd8902",
    urls = ["https://github.com/google/benchmark/archive/785e2c3158589e8ef48c59ba80e48d76bdbd8902.zip"],
)

# gRPC
http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "a439405b5691ad3f168f31a91db91cbab493e59502aa0893a7f5b6d95b59a663",
    strip_prefix = "grpc-8860eb34ab9183589a039a36a6e10a74b440c501",
    urls = ["https://github.com/grpc/grpc/archive/8860eb34ab9183589a039a36a6e10a74b440c501.tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# These bind()s are needed for the cc_grpc_library() rule to work.
bind(
    name = "grpc++_codegen_proto",
    actual = "@com_github_grpc_grpc//:grpc++_codegen_proto",
)

bind(
    name = "grpc_cpp_plugin",
    actual = "@com_github_grpc_grpc//:grpc_cpp_plugin",
)

# Used by prometheus-cpp.
local_repository(
    name = "net_zlib_zlib",
    path = "tools/zlib",
)

# Prometheus client library - used by Prometheus exporter.
http_archive(
    name = "com_github_jupp0r_prometheus_cpp",
    sha256 = "f24cb2a2d25bc31782e6424d20d8361563449b249be64f7c438e48f4efa741b7",
    strip_prefix = "prometheus-cpp-e9b8a774bb18ec8e432c7e943516d990a5abcd47",
    urls = ["https://github.com/jupp0r/prometheus-cpp/archive/e9b8a774bb18ec8e432c7e943516d990a5abcd47.zip"],
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
    sha256 = "3140ec1663cad975a38da9fce606fd80ca5978f89e87fc992c8fcd66cab0d1e4",
    strip_prefix = "curl-06f744d447af6648a00f0571116b5b901854abc4",
    urls = ["https://github.com/curl/curl/archive/06f744d447af6648a00f0571116b5b901854abc4.zip"],
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
    sha256 = "e3feb9d5f26de688d058056187a6901e89df0d4d1602fb3965455186c1e30df3",
    strip_prefix = "rapidjson-bfdcf4911047688fec49014d575433e2e5eb05be",
    urls = ["https://github.com/Tencent/rapidjson/archive/bfdcf4911047688fec49014d575433e2e5eb05be.zip"],
)
