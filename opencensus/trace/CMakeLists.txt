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

add_library(trace
            internal/annotation.cc
            internal/attribute_list.cc
            internal/attribute_value.cc
            internal/attribute_value_ref.cc
            internal/context_util.cc
            internal/link.cc
            internal/local_span_store.cc
            internal/local_span_store_impl.cc
            internal/message_event.cc
            internal/running_span_store.cc
            internal/running_span_store_impl.cc
            internal/sampler.cc
            internal/span.cc
            internal/span_context.cc
            internal/span_data.cc
            internal/span_exporter.cc
            internal/span_exporter_impl.cc
            internal/span_id.cc
            internal/span_impl.cc
            internal/status.cc
            internal/trace_config.cc
            internal/trace_config_impl.cc
            internal/trace_id.cc
            internal/trace_options.cc
            internal/with_span.cc)

add_library(${PROJECT_NAME}::trace ALIAS trace)

target_link_libraries(trace
                      PUBLIC common.random_lib absl::strings
                      PRIVATE absl::base
                              absl::memory
                              absl::synchronization
                              absl::time
                              absl::span)

target_include_directories(trace PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(trace.context_util internal/context_util.cc)

add_library(${PROJECT_NAME}::trace.context_util ALIAS trace.context_util)

target_link_libraries(trace.context_util PRIVATE trace context)

target_include_directories(trace.context_util PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

add_library(trace.with_span internal/with_span.cc)

add_library(${PROJECT_NAME}::trace.with_span ALIAS trace.with_span)

target_link_libraries(trace.with_span PRIVATE trace context)

target_include_directories(trace.with_span PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------

opencensus_test(trace.annotation_test internal/annotation_test.cc trace)

opencensus_test(trace.attribute_value_ref_test
                internal/attribute_value_ref_test.cc trace)

opencensus_test(trace.attribute_value_test internal/attribute_value_test.cc
                trace)

opencensus_test(trace.context_util_test
                internal/context_util_test.cc
                trace
                trace.context_util
                trace.with_span
                context)

opencensus_test(trace.link_test internal/link_test.cc trace)

opencensus_test(trace.local_span_store_test
                internal/local_span_store_test.cc
                trace
                absl::memory
                absl::synchronization)

opencensus_test(trace.running_span_store_test
                internal/running_span_store_test.cc
                trace
                absl::base
                absl::memory
                absl::synchronization)

opencensus_test(trace.sampler_test
                internal/sampler_test.cc
                trace
                absl::strings
                absl::synchronization
                absl::time)

opencensus_test(trace.span_test internal/span_test.cc trace absl::strings)

opencensus_test(trace.span_id_test internal/span_id_test.cc trace)

opencensus_test(trace.span_options_test
                internal/span_options_test.cc
                trace
                absl::strings
                absl::synchronization)

opencensus_test(trace.span_context_test
                internal/span_context_test.cc
                trace
                absl::strings
                absl::span)

opencensus_test(trace.span_exporter_test
                internal/span_exporter_test.cc
                trace
                absl::memory
                absl::strings
                absl::synchronization
                absl::time)

opencensus_test(trace.status_test
                internal/status_test.cc
                trace
                absl::strings
                gmock)

opencensus_test(trace.trace_config_test
                internal/trace_config_test.cc
                trace
                absl::time)

opencensus_test(trace.trace_options_test internal/trace_options_test.cc trace)

opencensus_test(trace.with_span_test
                internal/with_span_test.cc
                trace
                trace.with_span
                context)

# TODO: benchmarks
