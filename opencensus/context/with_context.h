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

#ifndef OPENCENSUS_CONTEXT_WITH_CONTEXT_H_
#define OPENCENSUS_CONTEXT_WITH_CONTEXT_H_

#include "opencensus/context/context.h"

namespace opencensus {
namespace context {

// WithContext is a scoped object that sets the current Context to the given
// one, until the WithContext object is destroyed.
//
// Because it changes the current (thread local) context, NEVER allocate a
// WithContext in one thread and deallocate in another. A simple way to ensure
// this is to only ever stack-allocate it.
class WithContext {
 public:
  explicit WithContext(const Context& ctx);
  explicit WithContext(Context&& ctx);
  ~WithContext();

 private:
  WithContext() = delete;
  WithContext(const WithContext&) = delete;
  WithContext(WithContext&&) = delete;
  WithContext& operator=(const WithContext&) = delete;
  WithContext& operator=(WithContext&&) = delete;

  Context swapped_context_;
#ifndef NDEBUG
  const Context* original_context_;
#endif
};

}  // namespace context
}  // namespace opencensus

#endif  // OPENCENSUS_CONTEXT_WITH_CONTEXT_H_
