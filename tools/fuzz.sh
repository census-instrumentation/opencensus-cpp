#!/bin/bash
# Copyright 2019, OpenCensus Authors
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
#
# Builds and runs fuzzers for a second (to process existing test cases).
if [[ ! -e tools/fuzz.sh ]]; then
  echo "This tool must be run from the topmost OpenCensus directory." >&2
  exit 1
fi
set -e
set -x
cmake -H. -B.build -DBUILD_TESTING=OFF -DCMAKE_CXX_COMPILER=clang-6.0 \
  -DFUZZER=-fsanitize=fuzzer \
  -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link,address -g -O2"
cmake --build .build
./.build/opencensus/tags/opencensus_tags_grpc_tags_bin_fuzzer \
  opencensus/tags/internal/grpc_tags_bin_corpus -runs=0
./.build/opencensus/trace/opencensus_trace_b3_fuzzer \
  opencensus/trace/internal/b3_corpus -runs=0
./.build/opencensus/trace/opencensus_trace_cloud_trace_context_fuzzer \
  opencensus/trace/internal/cloud_trace_context_corpus -runs=0
./.build/opencensus/trace/opencensus_trace_grpc_trace_bin_fuzzer \
  opencensus/trace/internal/grpc_trace_bin_corpus -runs=0
./.build/opencensus/trace/opencensus_trace_trace_context_fuzzer \
  opencensus/trace/internal/trace_context_corpus -runs=0
