// Copyright 2018, OpenCensus Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "benchmark/benchmark.h"
#include "opencensus/trace/span_context.h"

namespace opencensus {
namespace trace {
namespace {

constexpr char kXCTCFull[] =
    "12345678901234567890123456789012/18446744073709551615;o=1";
constexpr char kXCTCNoOptions[] =
    "12345678901234567890123456789012/18446744073709551615";
constexpr char kXCTCInvalidTraceId[] =
    "1234567890123456789012345678901x/18446744073709551615;o=1";

void BM_FromCloudTraceContext_Full(benchmark::State& state) {
  while (state.KeepRunning()) {
    SpanContext::FromCloudTraceContextHeader(kXCTCFull);
  }
}
BENCHMARK(BM_FromCloudTraceContext_Full);

void BM_FromCloudTraceContext_NoOptions(benchmark::State& state) {
  while (state.KeepRunning()) {
    SpanContext::FromCloudTraceContextHeader(kXCTCNoOptions);
  }
}
BENCHMARK(BM_FromCloudTraceContext_NoOptions);

void BM_FromCloudTraceContext_InvalidTraceId(benchmark::State& state) {
  while (state.KeepRunning()) {
    SpanContext::FromCloudTraceContextHeader(kXCTCInvalidTraceId);
  }
}
BENCHMARK(BM_FromCloudTraceContext_InvalidTraceId);

void BM_ToCloudTraceContext(benchmark::State& state) {
  auto ctx = SpanContext::FromCloudTraceContextHeader(kXCTCFull);
  while (state.KeepRunning()) {
    ctx.ToCloudTraceContextHeader();
  }
}
BENCHMARK(BM_ToCloudTraceContext);

}  // namespace
}  // namespace trace
}  // namespace opencensus

BENCHMARK_MAIN();
