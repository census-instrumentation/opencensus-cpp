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

opencensus_lib(common_hostname HDRS hostname.h SRCS hostname.cc DEPS absl::strings)

opencensus_lib(
  common_random
  HDRS
  random.h
  SRCS
  random.cc
  DEPS
  absl::base
  absl::synchronization
  absl::time)

add_subdirectory( grpc )

opencensus_lib(common_stats_object HDRS stats_object.h DEPS absl::time)

# Define NOMINMAX to fix build errors when compiling with MSVC.
target_compile_definitions(opencensus_common_stats_object
                           INTERFACE $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)

opencensus_lib(common_string_vector_hash HDRS string_vector_hash.h DEPS absl::hash)

opencensus_lib(common_varint HDRS varint.h SRCS varint.cc DEPS absl::strings)

find_package( protobuf REQUIRED )

opencensus_lib( common_timestamp HDRS timestamp.h SRCS timestamp.cc DEPS absl::time  )

target_link_libraries( opencensus_common_timestamp PUBLIC protobuf::libprotobuf )
# Tests.

opencensus_test(common_hostname_test hostname_test.cc common_hostname)

opencensus_test(common_random_test random_test.cc common_random)

opencensus_test(common_stats_object_test stats_object_test.cc
                common_stats_object absl::strings absl::span)

opencensus_test(common_varint_test varint_test.cc common_varint)

opencensus_test(common_timestamp_test timestamp_test.cc common_timestamp)

# Benchmarks.

opencensus_benchmark(common_random_benchmark random_benchmark.cc common_random)

opencensus_benchmark(common_timestamp_benchmark timestamp_benchmark.cc common_timestamp)
