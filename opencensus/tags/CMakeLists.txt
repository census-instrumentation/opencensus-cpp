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

add_library(tags internal/tag_key.cc internal/tag_map.cc)

add_library(${PROJECT_NAME}::tags ALIAS tags)

target_link_libraries(tags
                      PUBLIC absl::strings
                      PRIVATE common.hash_mix absl::base absl::synchronization)

target_include_directories(tags PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(tags.context_util internal/context_util.cc)

add_library(${PROJECT_NAME}::tags.context_util ALIAS tags.context_util)

target_link_libraries(tags.context_util PRIVATE tags context)

target_include_directories(tags.context_util PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(tags.with_tag_map internal/with_tag_map.cc)

add_library(${PROJECT_NAME}::tags.with_tag_map ALIAS tags.with_tag_map)

target_link_libraries(tags.with_tag_map PRIVATE tags context absl::strings)

target_include_directories(tags.with_tag_map PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

opencensus_test(tags.context_util_test
                internal/context_util_test.cc
                tags
                tags.context_util
                tags.with_tag_map
                context)

opencensus_test(tags.tag_key_test internal/tag_key_test.cc tags)

opencensus_test(tags.tag_map_test internal/tag_map_test.cc tags gmock)

opencensus_test(tags.with_tag_map_test
                internal/with_tag_map_test.cc
                tags
                tags.with_tag_map
                context)

# TODO: benchmarks
