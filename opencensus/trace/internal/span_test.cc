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

#include "opencensus/trace/span.h"

#include <cstdint>

#include "gtest/gtest.h"
#include "opencensus/trace/attribute_value_ref.h"
#include "opencensus/trace/exporter/attribute_value.h"
#include "opencensus/trace/exporter/span_data.h"
#include "opencensus/trace/internal/span_impl.h"
#include "opencensus/trace/span_id.h"
#include "opencensus/trace/trace_id.h"
#include "opencensus/trace/trace_options.h"

namespace opencensus {
namespace trace {

struct SpanTestPeer {
  static SpanId GetParentSpanId(Span* span) {
    return span->span_impl_for_test()->parent_span_id();
  }

  static void SetSampled(TraceOptions* opts, bool is_sampled) {
    opts->SetSampled(is_sampled);
  }

  static exporter::SpanData ToSpanData(Span* span) {
    return span->span_impl_for_test()->ToSpanData();
  }
};

namespace {

constexpr bool kRecordEvents = true;

TEST(SpanTest, SampledSpan) {
  AlwaysSampler sampler;
  auto span = Span::StartSpan("MySpan", /*parent=*/nullptr, {&sampler});
  EXPECT_TRUE(span.IsSampled());
  EXPECT_TRUE(span.IsRecording()) << "Sampled spans must be recording.";
}

TEST(SpanTest, RecordingButNotSampledSpan) {
  NeverSampler sampler;
  auto span =
      Span::StartSpan("MySpan", /*parent=*/nullptr, {&sampler, kRecordEvents});
  EXPECT_FALSE(span.IsSampled());
  EXPECT_TRUE(span.IsRecording());
}

TEST(SpanTest, NotRecordingAndNotSampledSpan) {
  NeverSampler sampler;
  auto span = Span::StartSpan("MySpan", /*parent=*/nullptr, {&sampler});
  EXPECT_FALSE(span.IsSampled());
  EXPECT_FALSE(span.IsRecording());
}

TEST(SpanTest, ChildInheritsSamplingFromParent) {
  AlwaysSampler sampler;
  auto root_span =
      Span::StartSpan("MyRootSpan", /*parent=*/nullptr, {&sampler});
  auto child_span = Span::StartSpan("MyChildSpan",
                                    /*parent=*/&root_span);
  EXPECT_TRUE(root_span.IsSampled());
  EXPECT_TRUE(child_span.IsSampled());
  EXPECT_EQ(root_span.context().trace_id(), child_span.context().trace_id());
  EXPECT_EQ(root_span.context().span_id(),
            SpanTestPeer::GetParentSpanId(&child_span));
}

TEST(SpanTest, AddAttributesLastValueWins) {
  auto span =
      Span::StartSpan("SpanName", /*parent=*/nullptr, {nullptr, kRecordEvents});
  span.AddAttributes({{"key", "value1"},
                      {"key", 123},
                      {"another_key", "another_value"},
                      {"key", "value2"},
                      {"bool_key", true}});
  auto data = SpanTestPeer::ToSpanData(&span);
  span.End();
  EXPECT_EQ("value2", data.attributes().at("key").string_value())
      << "Last value wins.";
  EXPECT_EQ("another_value",
            data.attributes().at("another_key").string_value());
}

TEST(SpanTest, AddAnnotationLastAttributeWins) {
  auto span =
      Span::StartSpan("SpanName", /*parent=*/nullptr, {nullptr, kRecordEvents});
  span.AddAnnotation("Annotation text.", {{"key", "value1"},
                                          {"key", 123},
                                          {"another_key", "another_value"},
                                          {"key", "value2"},
                                          {"bool_key", true}});
  auto data = SpanTestPeer::ToSpanData(&span);
  span.End();
  EXPECT_EQ("value2", data.annotations()
                          .events()[0]
                          .event()
                          .attributes()
                          .at("key")
                          .string_value())
      << "Last value wins.";
  EXPECT_EQ("another_value", data.annotations()
                                 .events()[0]
                                 .event()
                                 .attributes()
                                 .at("another_key")
                                 .string_value());
}

TEST(SpanTest, ParentLinksFromOptions) {
  auto parent0 =
      Span::StartSpan("Parent0", /*parent=*/nullptr, {nullptr, kRecordEvents});
  auto parent1 =
      Span::StartSpan("Parent1", /*parent=*/nullptr, {nullptr, kRecordEvents});
  auto span = Span::StartSpan("MyRootSpan",
                              /*parent=*/nullptr,
                              {nullptr, kRecordEvents, {&parent0, &parent1}});
  // Check that StartSpan added parent links to span.
  {
    auto data = SpanTestPeer::ToSpanData(&span);
    EXPECT_FALSE(data.parent_span_id().IsValid())
        << "Span has no parent in its own trace.";
    auto expect_link = [](const Span& parent, const exporter::Link& link) {
      EXPECT_EQ(exporter::Link::Type::kParentLinkedSpan, link.type());
      EXPECT_EQ(parent.context().trace_id(), link.trace_id());
      EXPECT_EQ(parent.context().span_id(), link.span_id());
      EXPECT_EQ(0, link.attributes().size());
    };
    ASSERT_EQ(2, data.links().size());
    expect_link(parent0, data.links()[0]);
    expect_link(parent1, data.links()[1]);
  }
  // Check that StartSpan added child links to the parents.
  for (Span* parent : {&parent0, &parent1}) {
    auto data = SpanTestPeer::ToSpanData(parent);
    ASSERT_EQ(1, data.links().size());
    auto& link = data.links()[0];
    EXPECT_EQ(exporter::Link::Type::kChildLinkedSpan, link.type());
    EXPECT_EQ(span.context().trace_id(), link.trace_id());
    EXPECT_EQ(span.context().span_id(), link.span_id());
    EXPECT_EQ(0, link.attributes().size());
  }
}

TEST(SpanTest, EverythingASpanCanDo) {
  constexpr uint8_t trace_id[] = {1, 2,  3,  4,  5,  6,  7,  8,
                                  9, 10, 11, 12, 13, 14, 15, 16};
  constexpr uint8_t span_id[] = {1, 2, 3, 4, 5, 6, 7, 8};
  auto ctx = SpanContext(TraceId(trace_id), SpanId(span_id));

  AlwaysSampler sampler;
  auto related_span =
      Span::StartSpan("RelatedSpan", /*parent=*/nullptr, {&sampler});
  StartSpanOptions opts = {&sampler, kRecordEvents, {&related_span}};
  auto span = ::opencensus::trace::Span::StartSpan("MyRootSpan",
                                                   /*parent=*/nullptr, opts);

  AttributeValueRef av("value1");
  span.AddAttribute("key1", av);
  span.AddAttribute("key2", "value2");
  span.AddAttributes({{"key3", "value3"}, {"key4", 123}, {"key5", false}});

  span.AddAnnotation("anno1");
  span.AddAnnotation(
      "anno2", {{"str_attr", "hello"}, {"int_attr", 123}, {"bool_attr", true}});

  span.AddSentMessageEvent(2, 3, 4);
  span.AddReceivedMessageEvent(3, 4, 5);

  span.AddParentLink(ctx);
  span.AddParentLink(ctx, {{"attrib1", "value1"}});
  span.AddChildLink(ctx);
  span.AddChildLink(ctx, {{"attrib2", 123}});

  span.SetStatus(StatusCode::DEADLINE_EXCEEDED, "desc");

  EXPECT_TRUE(span.context().IsValid());
  span.End();

  // TODO: Check recorded data.
}

TEST(SpanTest, ChildInheritsTraceOption) {
  constexpr uint8_t trace_id[] = {1, 2,  3,  4,  5,  6,  7,  8,
                                  9, 10, 11, 12, 13, 14, 15, 16};
  constexpr uint8_t span_id[] = {1, 0, 0, 0, 0, 0, 0, 11};
  // Create a parent context with TraceOptions set to sampled.
  TraceOptions trace_options;
  SpanTestPeer::SetSampled(&trace_options, true);
  SpanContext parent_ctx1{TraceId(trace_id), SpanId(span_id),
                          TraceOptions(trace_options)};

  // Create a child Span with default sampling.
  auto span1 = Span::StartSpanWithRemoteParent("Span1", parent_ctx1);
  EXPECT_TRUE(span1.IsSampled());
}

TEST(SpanTest, ForceSamplingOnViaTraceConfig) {
  // No sampling requested, but trace_params forces it.
  TraceConfig::SetCurrentTraceParams(
      TraceParams{32, 32, 128, 128, ProbabilitySampler(1.0)});
  for (int i = 0; i < 1000; ++i) {
    auto span = Span::StartSpan("SpanName");
    EXPECT_TRUE(span.IsSampled());
    span.End();
  }
}

TEST(SpanTest, ForceSamplingOffViaTraceConfig) {
  // No sampling requested, but trace_params forces it.
  TraceConfig::SetCurrentTraceParams(
      TraceParams{32, 32, 128, 128, ProbabilitySampler(0.0)});
  for (int i = 0; i < 1000; ++i) {
    auto span = Span::StartSpan("SpanName");
    EXPECT_FALSE(span.IsSampled());
    span.End();
  }
}

TEST(SpanTest, CheckSpanData) {
  AlwaysSampler sampler;
  auto current_span = Span::StartSpan("test_span", nullptr, {&sampler});

  current_span.AddAttribute("test1", "attribute1");
  current_span.AddAttribute("test2", true);
  current_span.AddAttribute("test3", 333);

  const exporter::SpanData data = SpanTestPeer::ToSpanData(&current_span);

  EXPECT_EQ("test_span", data.name());
  EXPECT_EQ(SpanId(), data.parent_span_id()) << "No parent.";
  EXPECT_EQ(0, data.annotations().dropped_events_count());
  EXPECT_EQ(0, data.message_events().dropped_events_count());
  EXPECT_EQ(0, data.num_links_dropped());
  EXPECT_EQ(0, data.num_attributes_dropped());
  EXPECT_EQ(false, data.has_ended());
  EXPECT_TRUE(data.status().ok());
  EXPECT_EQ(false, data.has_remote_parent());

  std::unordered_map<std::string, exporter::AttributeValue> attributes =
      data.attributes();
  EXPECT_EQ("attribute1", attributes.at("test1").string_value());
  EXPECT_EQ(true, attributes.at("test2").bool_value());
  EXPECT_EQ(333, attributes.at("test3").int_value());
}

}  // namespace
}  // namespace trace
}  // namespace opencensus
