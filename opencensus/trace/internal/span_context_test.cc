// Copyright 2017, OpenCensus Authors
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

#include "opencensus/trace/span_context.h"
#include "opencensus/trace/span_id.h"
#include "opencensus/trace/trace_id.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace opencensus {
namespace trace {
namespace {

MATCHER(IsValid, "is a valid SpanContext") { return arg.IsValid(); }
MATCHER(IsInvalid, "is an invalid SpanContext") { return !arg.IsValid(); }

constexpr uint8_t trace_id[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6};
constexpr uint8_t span_id[] = {10, 2, 3, 4, 5, 6, 7, 8};

TEST(SpanContextTest, IsValid) {
  SpanContext ctx1;
  SpanContext ctx2{TraceId(trace_id), SpanId(span_id)};
  SpanContext ctx3{TraceId(), SpanId(span_id)};
  SpanContext ctx4{TraceId(trace_id), SpanId()};

  EXPECT_FALSE(ctx1.IsValid()) << "A blank context shouldn't be valid.";
  EXPECT_TRUE(ctx2.IsValid());
  EXPECT_FALSE(ctx3.IsValid());
  EXPECT_FALSE(ctx4.IsValid());
}

TEST(SpanContextTest, Equality) {
  constexpr uint8_t span_id2[] = {1, 20, 3, 4, 5, 6, 7, 8};
  constexpr uint8_t opts[] = {1};

  SpanContext ctx1{TraceId(trace_id), SpanId(span_id)};
  SpanContext ctx2{TraceId(trace_id), SpanId(span_id2)};
  SpanContext ctx3{TraceId(trace_id), SpanId(span_id)};
  SpanContext ctx4{TraceId(trace_id), SpanId(span_id), TraceOptions(opts)};

  EXPECT_EQ(ctx1, ctx1);
  EXPECT_NE(ctx1, ctx2);
  EXPECT_EQ(ctx1, ctx3) << "Same contents, different objects.";
  EXPECT_EQ(ctx1, ctx4) << "Comparison ignores options.";
  EXPECT_FALSE(ctx1 == ctx2);
}

TEST(SpanContextTest, ToString) {
  constexpr uint8_t opts[] = {1};
  SpanContext ctx{TraceId(trace_id), SpanId(span_id), TraceOptions(opts)};
  EXPECT_EQ("01020304050607080900010203040506-0a02030405060708-01",
            ctx.ToString());
}

// Tests for "X-Cloud-Trace-Context:" parser.
TEST(SpanContextTest, ParseFull) {
  constexpr char header[] = "01020304050607081112131415161718/123;o=1";
  SpanContext ctx = SpanContext::FromCloudTraceContextHeader(header);
  EXPECT_THAT(ctx, IsValid());
  EXPECT_EQ("01020304050607081112131415161718-000000000000007b-01",
            ctx.ToString());
  EXPECT_EQ(header, ctx.ToCloudTraceContextHeader());
}

TEST(SpanContextTest, ParseNoOptions) {
  constexpr char header[] = "01020304050607081112131415161718/456";
  SpanContext ctx = SpanContext::FromCloudTraceContextHeader(header);
  EXPECT_THAT(ctx, IsValid());
  EXPECT_EQ("01020304050607081112131415161718-00000000000001c8-00",
            ctx.ToString());
  EXPECT_EQ("01020304050607081112131415161718/456;o=0",
            ctx.ToCloudTraceContextHeader())
      << "missing option is canonicalized to o=0";
}

TEST(SpanContextTest, ParseMaxValues) {
  constexpr char header[] =
      "ffffffffffffffffffffffffffffffff/18446744073709551615;o=3";
  SpanContext ctx = SpanContext::FromCloudTraceContextHeader(header);
  EXPECT_THAT(ctx, IsValid());
  EXPECT_EQ("ffffffffffffffffffffffffffffffff-ffffffffffffffff-01",
            ctx.ToString());
  EXPECT_EQ("ffffffffffffffffffffffffffffffff/18446744073709551615;o=1",
            ctx.ToCloudTraceContextHeader())
      << "o=3 is canonicalized to o=1";
}

TEST(SpanContextTest, ExpectedFailures) {
#define INVALID(str) \
  EXPECT_THAT(SpanContext::FromCloudTraceContextHeader(str), IsInvalid())

  INVALID("");
  INVALID("1234567890123456789012345678901") << "too short.";
  INVALID("12345678901234567890123456789012") << "too short.";
  INVALID("12345678901234567890123456789012/") << "too short.";
  INVALID("1xyz5678901234567890123456789012/1") << "trace_id not hex.";
  INVALID("123456789012345678901234567890123/1") << "trace_id too long.";
  INVALID("12345678901234567890123456789012/;o=1") << "missing span_id.";
  INVALID("12345678901234567890123456789012/1xy2;o=1")
      << "span_id must be decimal.";
  INVALID("12345678901234567890123456789012/123/123") << "too many slashes.";
  INVALID("12345678901234567890123456789012/18446744073709551617;o=1")
      << "span_id is too large. (uint64max + 1)";
  INVALID("12345678901234567890123456789012/456;") << "missing options.";
  INVALID("12345678901234567890123456789012/456;o=")
      << "missing options value.";
  INVALID("12345678901234567890123456789012/456;o=4")
      << "options value too large.";
  INVALID("12345678901234567890123456789012/456;o01") << "malformed options.";
  INVALID("12345678901234567890123456789012/456;o1") << "malformed options.";
  INVALID("00000000000000000000000000000000/456;o=1")
      << "zero is an invalid trace_id.";
  INVALID("12345678901234567890123456789012/0;o=1")
      << "zero is an invalid span_id.";
  INVALID("12345678901234567890123456789012/-123;o=1")
      << "negative span_id is invalid.";
#undef INVALID
}

}  // namespace
}  // namespace trace
}  // namespace opencensus
