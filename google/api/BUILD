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
    name = "annotations",
    srcs = ["annotations.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [":http"],
)

cc_grpc_library(
    name = "distribution",
    srcs = ["distribution.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [":annotations"],
)

cc_grpc_library(
    name = "http",
    srcs = ["http.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [],
)

cc_grpc_library(
    name = "label",
    srcs = ["label.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [],
)

cc_grpc_library(
    name = "metric",
    srcs = ["metric.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [":label"],
)

cc_grpc_library(
    name = "monitored_resource",
    srcs = ["monitored_resource.proto"],
    proto_only = False,
    use_external = True,
    well_known_protos = True,
    deps = [":label"],
)
