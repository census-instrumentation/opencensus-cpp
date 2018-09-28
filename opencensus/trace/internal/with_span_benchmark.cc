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

#include "opencensus/trace/with_span.h"

#include "benchmark/benchmark.h"
#include "opencensus/trace/span.h"

namespace opencensus {
namespace context {
class ContextTestPeer {
 public:
  static const opencensus::trace::Span& CurrentSpan() {
    return Context::InternalMutableCurrent()->span_;
  }
};
}  // namespace context
namespace trace {
namespace {

using opencensus::context::ContextTestPeer;

void BM_WithSpan(benchmark::State& state) {
  auto span = Span::StartSpan("MySpan");
  for (auto _ : state) {
    WithSpan ws(span);
  }
  span.End();
}
BENCHMARK(BM_WithSpan);

void BM_WithSpanConditionFalse(benchmark::State& state) {
  auto span = Span::StartSpan("MySpan");
  for (auto _ : state) {
    WithSpan ws(span, false);
  }
  span.End();
}
BENCHMARK(BM_WithSpanConditionFalse);

void BM_WithSpanConstructAndCopy(benchmark::State& state) {
  for (auto _ : state) {
    auto span = Span::StartSpan("MySpan");
    WithSpan ws(span);
    ContextTestPeer::CurrentSpan().End();
  }
}
BENCHMARK(BM_WithSpanConstructAndCopy);

void BM_WithSpanConstructAndMove(benchmark::State& state) {
  for (auto _ : state) {
    auto span = Span::StartSpan("MySpan");
    WithSpan ws(std::move(span));
    ContextTestPeer::CurrentSpan().End();
  }
}
BENCHMARK(BM_WithSpanConstructAndMove);

}  // namespace
}  // namespace trace
}  // namespace opencensus

BENCHMARK_MAIN();
