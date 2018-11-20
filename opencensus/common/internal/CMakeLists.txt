# ~~~
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
# ~~~

add_library(common.hash_mix INTERFACE)
target_include_directories(common.hash_mix INTERFACE ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(common.random_lib random.cc)

target_link_libraries(common.random_lib
                      PUBLIC absl::base
                      PRIVATE absl::synchronization absl::time)

target_include_directories(common.random_lib PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(common.stats_object INTERFACE)
target_include_directories(common.stats_object
                           INTERFACE ${OPENCENSUS_INCLUDE_DIR})
target_link_libraries(common.stats_object INTERFACE absl::time)

# ----------------------------------------------------------------------

add_library(common.string_vector_hash INTERFACE)
target_include_directories(common.string_vector_hash
                           INTERFACE ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

opencensus_test(common.random_test random_test.cc common.random_lib)

opencensus_test(common.stats_object_test
                stats_object_test.cc
                common.stats_object
                absl::strings
                absl::span
                gmock)

# TODO: random_benchmark
