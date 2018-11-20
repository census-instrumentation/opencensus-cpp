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

add_library(context internal/context.cc internal/with_context.cc)

add_library(${PROJECT_NAME}::context ALIAS context)

target_link_libraries(context PUBLIC absl::strings PRIVATE tags trace)

target_include_directories(context PUBLIC ${OPENCENSUS_INCLUDE_DIR})

# ----------------------------------------------------------------------

opencensus_test(context.context_test
                internal/context_test.cc
                context
                tags.context_util
                tags.with_tag_map
                trace.context_util
                trace.with_span)

opencensus_test(context.with_context_test internal/with_context_test.cc context)

# ----------------------------------------------------------------------

# TODO: context_benchmark
