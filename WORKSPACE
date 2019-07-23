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
  urls = ["https://github.com/abseil/abseil-cpp/archive/f3840bc5e33ce4932e35986cf3718450c6f02af2.zip"],  # 2019-07-19T17:04:45Z
  strip_prefix = "abseil-cpp-f3840bc5e33ce4932e35986cf3718450c6f02af2",
  sha256 = "ee568c74d4d94795dd0c17cee0a5ed30383fe53119cf191d8b68e626e0ed6bf6",
)

# GoogleTest framework.
# Only needed for tests, not to build the OpenCensus library.
http_archive(
  name = "com_google_googletest",
  urls = ["https://github.com/google/googletest/archive/b77e5c76252bac322bb82c5b444f050bd0d92451.zip"],  # 2019-07-18T19:27:51Z
  strip_prefix = "googletest-b77e5c76252bac322bb82c5b444f050bd0d92451",
  sha256 = "c53f3770c9c008ba208a20770c8af2e81fa5b29bfd688a6e7e63c7dad625cac7",
)

# Google Benchmark library.
# Only needed for benchmarks, not to build the OpenCensus library.
http_archive(
  name = "com_github_google_benchmark",
  urls = ["https://github.com/google/benchmark/archive/8e48105d465c586068dd8e248fe75a8971c6ba3a.zip"],  # 2019-07-22T12:42:12Z
  strip_prefix = "benchmark-8e48105d465c586068dd8e248fe75a8971c6ba3a",
  sha256 = "dedc0f15e654b42d4036768c33d7f2ab5c3020939f0be052ae0a0b76a4830e62",
)

# gRPC
http_archive(
  name = "com_github_grpc_grpc",
  urls = ["https://github.com/grpc/grpc/archive/809e7c951358a80182d7126b255c3a40881fb3fa.zip"],  # 2019-07-23T00:43:25Z
  strip_prefix = "grpc-809e7c951358a80182d7126b255c3a40881fb3fa",
  sha256 = "54130a7fa3dae57ed148f24cddcc91ff56e8023ed3d1e44cff4e1a922406087d",
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
  urls = ["https://github.com/jupp0r/prometheus-cpp/archive/d83dd68e496e024ae0f1f0c19ac2ab0d27330330.zip"],  # 2019-07-03T18:56:43Z
  strip_prefix = "prometheus-cpp-d83dd68e496e024ae0f1f0c19ac2ab0d27330330",
  sha256 = "040653580faea4d10a3fc617f1276ce32d3e9d48277e8bf701a317d34d5f8e29",
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
  urls = ["https://github.com/curl/curl/archive/23c99f60babd64164776c8bef1525fa411f8bed1.zip"],  # 2019-07-21T22:28:55Z
  strip_prefix = "curl-23c99f60babd64164776c8bef1525fa411f8bed1",
  sha256 = "1fa557287b6702fc2104c48eb7d4a9e66fe509d68de23425f577ef1f2e001c29",
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
  urls = ["https://github.com/Tencent/rapidjson/archive/d87b698d0fcc10a5f632ecbc80a9cb2a8fa094a5.zip"],  # 2019-06-28T02:37:13Z
  strip_prefix = "rapidjson-d87b698d0fcc10a5f632ecbc80a9cb2a8fa094a5",
  sha256 = "5428830eae1e19d0436e64b214c727aab4a33379129cdede11b0507974ad134f",
)

# Google APIs - used by Stackdriver exporter.
http_archive(
  name = "com_google_googleapis",
  urls = ["https://github.com/googleapis/googleapis/archive/6e3b55e26bf5a9f7874b6ba1411a0cc50cb87a48.zip"],  # 2019-07-23T00:01:26Z
  strip_prefix = "googleapis-6e3b55e26bf5a9f7874b6ba1411a0cc50cb87a48",
  sha256 = "150be57ff83646e5652e03683c949f0830d9a0e73ef787786864210e45537fe0",
)

load("@com_google_googleapis//:repository_rules.bzl", "switched_rules_by_language")

switched_rules_by_language(
    name = "com_google_googleapis_imports",
    cc = True,
    grpc = True,
)
