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

add_library(stats INTERFACE)

add_library(${PROJECT_NAME}::stats ALIAS stats)

target_link_libraries(stats INTERFACE stats.core stats.recording)

target_include_directories(stats INTERFACE ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(stats.test_utils testing/test_utils.cc)

add_library(${PROJECT_NAME}::stats.test_utils ALIAS stats.test_utils)

target_link_libraries(stats.test_utils
                      PRIVATE stats.core absl::memory absl::time)

target_include_directories(stats.test_utils PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(stats.core
            internal/aggregation.cc
            internal/aggregation_window.cc
            internal/bucket_boundaries.cc
            internal/delta_producer.cc
            internal/distribution.cc
            internal/measure.cc
            internal/measure_data.cc
            internal/measure_descriptor.cc
            internal/measure_registry.cc
            internal/measure_registry_impl.cc
            internal/set_aggregation_window.cc
            internal/stats_exporter.cc
            internal/stats_manager.cc
            internal/view.cc
            internal/view_data.cc
            internal/view_data_impl.cc
            internal/view_descriptor.cc)

target_link_libraries(stats.core
                      PUBLIC absl::base
                      PRIVATE common.stats_object
                              common.string_vector_hash
                              tags
                              absl::memory
                              absl::strings
                              absl::synchronization
                              absl::time
                              absl::optional
                              absl::span)

target_include_directories(stats.core PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(stats.recording internal/recording.cc)

target_link_libraries(stats.recording
                      PRIVATE stats.core
                              tags
                              tags.context_util
                              absl::strings
                              absl::time)

target_include_directories(stats.recording PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------

opencensus_test(stats.debug_string_test
                internal/debug_string_test.cc
                stats.core
                absl::time
                gmock)

opencensus_test(stats.distribution_test
                internal/distribution_test.cc
                stats.core
                stats.test_utils
                gmock)

opencensus_test(stats.bucket_boundaries_test
                internal/bucket_boundaries_test.cc
                stats.core
                gmock)

opencensus_test(stats.measure_data_test
                internal/measure_data_test.cc
                stats.core
                stats.test_utils
                absl::span
                gmock)

opencensus_test(stats.measure_registry_test
                internal/measure_registry_test.cc
                stats.core
                absl::strings
                gmock)

opencensus_test(stats.stats_exporter_test
                internal/stats_exporter_test.cc
                stats.core
                stats.recording
                absl::memory
                absl::time
                gmock)

opencensus_test(stats.stats_manager_test
                internal/stats_manager_test.cc
                stats.core
                stats.recording
                stats.test_utils
                tags
                tags.with_tag_map
                gmock)

opencensus_test(stats.view_data_impl_test
                internal/view_data_impl_test.cc
                stats.core
                absl::time
                gmock)

# TODO: benchmarks
