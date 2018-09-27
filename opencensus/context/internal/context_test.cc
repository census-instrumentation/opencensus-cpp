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

#include "opencensus/context/context.h"

#include <iostream>

#include "gtest/gtest.h"

// Not in namespace ::opencensus::context in order to better reflect what user
// code should look like.

namespace {

void LogCurrentContext() {
  const std::string s = opencensus::context::Context::Current().DebugString();
  std::cout << "  current: " << s << "\n";
}

TEST(ContextTest, DefaultContext) { LogCurrentContext(); }

void Callback1() {
  std::cout << " inside function\n";
  LogCurrentContext();
}

TEST(ContextTest, Wrap) {
  std::function<void()> fn =
      opencensus::context::Context::Current().Wrap(Callback1);
  fn();
}

TEST(ContextTest, WrapDoesNotLeak) {
  {
    std::function<void()> fn =
        opencensus::context::Context::Current().Wrap(Callback1);
  }
  // We never call fn().
}

TEST(ContextTest, WrappedFnIsCopiable) {
  std::function<void()> fn2;
  {
    std::function<void()> fn1 =
        opencensus::context::Context::Current().Wrap(Callback1);
    fn2 = fn1;
    fn1();
  }
  fn2();
}

}  // namespace
