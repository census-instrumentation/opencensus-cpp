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

opencensus_lib(trace
               PUBLIC
               SRCS
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
               internal/with_span.cc
               DEPS
               common_random
               absl::strings
               absl::base
               absl::memory
               absl::synchronization
               absl::time
               absl::span)

opencensus_lib(trace_context_util
               PUBLIC
               SRCS
               internal/context_util.cc
               DEPS
               trace
               context)

opencensus_lib(trace_with_span
               PUBLIC
               SRCS
               internal/with_span.cc
               DEPS
               trace
               context)

# ----------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------

opencensus_test(trace_annotation_test internal/annotation_test.cc trace)

opencensus_test(trace_attribute_value_ref_test
                internal/attribute_value_ref_test.cc trace)

opencensus_test(trace_attribute_value_test internal/attribute_value_test.cc
                trace)

opencensus_test(trace_context_util_test
                internal/context_util_test.cc
                trace
                trace_context_util
                trace_with_span
                context)

opencensus_test(trace_link_test internal/link_test.cc trace)

opencensus_test(trace_local_span_store_test
                internal/local_span_store_test.cc
                trace
                absl::memory
                absl::synchronization)

opencensus_test(trace_running_span_store_test
                internal/running_span_store_test.cc
                trace
                absl::base
                absl::memory
                absl::synchronization)

opencensus_test(trace_sampler_test
                internal/sampler_test.cc
                trace
                absl::strings
                absl::synchronization
                absl::time)

opencensus_test(trace_span_test internal/span_test.cc trace absl::strings)

opencensus_test(trace_span_id_test internal/span_id_test.cc trace)

opencensus_test(trace_span_options_test
                internal/span_options_test.cc
                trace
                absl::strings
                absl::synchronization)

opencensus_test(trace_span_context_test
                internal/span_context_test.cc
                trace
                absl::strings
                absl::span)

opencensus_test(trace_span_exporter_test
                internal/span_exporter_test.cc
                trace
                absl::memory
                absl::strings
                absl::synchronization
                absl::time)

opencensus_test(trace_status_test internal/status_test.cc trace absl::strings)

opencensus_test(trace_trace_config_test
                internal/trace_config_test.cc
                trace
                absl::time)

opencensus_test(trace_trace_options_test internal/trace_options_test.cc trace)

opencensus_test(trace_with_span_test
                internal/with_span_test.cc
                trace
                trace_with_span
                context)

# TODO: benchmarks
