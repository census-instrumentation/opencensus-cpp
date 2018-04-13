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

licenses(["notice"])  # Apache License 2.0

package(default_visibility = [
    "//google:__subpackages__",
    "//opencensus:__subpackages__",
])

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

cc_grpc_library(
    name = "trace_proto",
    srcs = ["trace.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [
        "//google/api:annotations",
        "//google/rpc:status",
    ],
)

cc_grpc_library(
    name = "tracing_proto",
    srcs = ["tracing.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [
        ":trace_proto",
        "//google/api:annotations",
    ],
)
