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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "opencensus/stats/internal/delta_producer.h"
#include "opencensus/stats/measure.h"
#include "opencensus/stats/measure_registry.h"
#include "opencensus/stats/view.h"

namespace opencensus {
namespace stats {
namespace {

constexpr char kFirstMeasureId[] = "first_measure_name";
constexpr char kSecondMeasureId[] = "second_measure_name";

MeasureDouble FirstMeasure() {
  static MeasureDouble measure = MeasureRegistry::RegisterDouble(
      kFirstMeasureId, "ops", "Usage of resource 1.");
  return measure;
}

MeasureInt SecondMeasure() {
  static MeasureInt measure = MeasureRegistry::RegisterInt(
      kSecondMeasureId, "ops", "Usage of resource 2.");
  return measure;
}

// These tests use the public stats interfaces, View and Measure--these are a
// thin layer around the StatsManager.
class StatsManagerTest : public ::testing::Test {
 protected:
  void SetUp() {
    // Access measures to be sure they are initialized.
    FirstMeasure();
    SecondMeasure();
    DeltaProducer::Get()->Flush();
  }

  const std::string key1_ = "key1";
  const std::string key2_ = "key2";
  const std::string key3_ = "key3";
};

TEST_F(StatsManagerTest, Count) {
  ViewDescriptor view_descriptor = ViewDescriptor()
                                       .set_measure(kFirstMeasureId)
                                       .set_name("count")
                                       .set_aggregation(Aggregation::Count())
                                       .add_column(key1_)
                                       .add_column(key2_);
  View view(view_descriptor);
  ASSERT_EQ(ViewData::Type::kInt64, view.GetData().type());
  EXPECT_TRUE(view.GetData().int_data().empty());

  // Stats under a different measure should be ignored.
  ExperimentalDeltaProducerRecord({{SecondMeasure(), 1}});
  DeltaProducer::Get()->Flush();
  EXPECT_TRUE(view.GetData().int_data().empty());

  ExperimentalDeltaProducerRecord(
      {{FirstMeasure(), 2.0}, {FirstMeasure(), 3.0}});
  ExperimentalDeltaProducerRecord(
      {{FirstMeasure(), 4.0}},
      {{key1_, "value1"}, {key2_, "value2"}, {key3_, "value3"}});
  DeltaProducer::Get()->Flush();
  const opencensus::stats::ViewData data = view.GetData();
  EXPECT_THAT(
      data.int_data(),
      ::testing::UnorderedElementsAre(
          ::testing::Pair(::testing::ElementsAre("", ""), 2.0),
          ::testing::Pair(::testing::ElementsAre("value1", "value2"), 1.0)));
}

TEST_F(StatsManagerTest, Sum) {
  ViewDescriptor view_descriptor = ViewDescriptor()
                                       .set_measure(kSecondMeasureId)
                                       .set_name("sum")
                                       .set_aggregation(Aggregation::Sum())
                                       .add_column(key1_)
                                       .add_column(key2_);
  View view(view_descriptor);
  ASSERT_EQ(ViewData::Type::kDouble, view.GetData().type());
  EXPECT_TRUE(view.GetData().double_data().empty());

  // Stats under a different measure should be ignored.
  ExperimentalDeltaProducerRecord({{FirstMeasure(), 1.0}});
  DeltaProducer::Get()->Flush();
  EXPECT_TRUE(view.GetData().double_data().empty());

  ExperimentalDeltaProducerRecord({{SecondMeasure(), 2}, {SecondMeasure(), 3}});
  ExperimentalDeltaProducerRecord(
      {{SecondMeasure(), 4}},
      {{key1_, "value1"}, {key2_, "value2"}, {key3_, "value3"}});
  DeltaProducer::Get()->Flush();
  const opencensus::stats::ViewData data = view.GetData();
  EXPECT_THAT(
      data.double_data(),
      ::testing::UnorderedElementsAre(
          ::testing::Pair(::testing::ElementsAre("", ""), 5.0),
          ::testing::Pair(::testing::ElementsAre("value1", "value2"), 4.0)));
}

TEST_F(StatsManagerTest, Distribution) {
  ViewDescriptor view_descriptor =
      ViewDescriptor()
          .set_measure(kSecondMeasureId)
          .set_name("distribution")
          .set_aggregation(
              Aggregation::Distribution(BucketBoundaries::Explicit({10})))
          .add_column(key1_)
          .add_column(key2_);
  View view(view_descriptor);
  ASSERT_EQ(ViewData::Type::kDistribution, view.GetData().type());
  EXPECT_TRUE(view.GetData().distribution_data().empty());

  // Stats under a different measure should be ignored.
  ExperimentalDeltaProducerRecord({{FirstMeasure(), 1.0}});
  DeltaProducer::Get()->Flush();
  EXPECT_TRUE(view.GetData().distribution_data().empty());

  ExperimentalDeltaProducerRecord(
      {{SecondMeasure(), 5}, {SecondMeasure(), 15}});
  ExperimentalDeltaProducerRecord(
      {{SecondMeasure(), 5}},
      {{key1_, "value1"}, {key2_, "value2"}, {key3_, "value3"}});
  DeltaProducer::Get()->Flush();
  const opencensus::stats::ViewData data = view.GetData();
  EXPECT_EQ(2, data.distribution_data().size());
  EXPECT_THAT(data.distribution_data().find({"", ""})->second.bucket_counts(),
              ::testing::ElementsAre(1, 1));
  EXPECT_THAT(data.distribution_data()
                  .find({"value1", "value2"})
                  ->second.bucket_counts(),
              ::testing::ElementsAre(1, 0));
}

// TODO: Test window expiration if we add a simulated clock.
TEST_F(StatsManagerTest, IntervalCount) {
  ViewDescriptor view_descriptor = ViewDescriptor()
                                       .set_measure(kFirstMeasureId)
                                       .set_name("interval-count")
                                       .set_aggregation(Aggregation::Count())
                                       .add_column(key1_)
                                       .add_column(key2_);
  SetAggregationWindow(AggregationWindow::Interval(absl::Minutes(1)),
                       &view_descriptor);
  View view(view_descriptor);
  ASSERT_EQ(ViewData::Type::kDouble, view.GetData().type());
  EXPECT_TRUE(view.GetData().double_data().empty());

  // Stats under a different measure should be ignored.
  ExperimentalDeltaProducerRecord({{SecondMeasure(), 1}});
  DeltaProducer::Get()->Flush();
  EXPECT_TRUE(view.GetData().double_data().empty());

  ExperimentalDeltaProducerRecord(
      {{FirstMeasure(), 2.0}, {FirstMeasure(), 3.0}});
  ExperimentalDeltaProducerRecord(
      {{FirstMeasure(), 4.0}},
      {{key1_, "value1"}, {key2_, "value2"}, {key3_, "value3"}});
  DeltaProducer::Get()->Flush();
  const opencensus::stats::ViewData data = view.GetData();
  EXPECT_THAT(
      data.double_data(),
      ::testing::UnorderedElementsAre(
          ::testing::Pair(::testing::ElementsAre("", ""), 2.0),
          ::testing::Pair(::testing::ElementsAre("value1", "value2"), 1.0)));
}

TEST_F(StatsManagerTest, IntervalSum) {
  ViewDescriptor view_descriptor = ViewDescriptor()
                                       .set_measure(kSecondMeasureId)
                                       .set_name("interval-sum")
                                       .set_aggregation(Aggregation::Sum())
                                       .add_column(key1_)
                                       .add_column(key2_);
  SetAggregationWindow(AggregationWindow::Interval(absl::Minutes(1)),
                       &view_descriptor);
  View view(view_descriptor);
  ASSERT_EQ(ViewData::Type::kDouble, view.GetData().type());
  EXPECT_TRUE(view.GetData().double_data().empty());

  // Stats under a different measure should be ignored.
  ExperimentalDeltaProducerRecord({{FirstMeasure(), 1.0}});
  DeltaProducer::Get()->Flush();
  EXPECT_TRUE(view.GetData().double_data().empty());

  ExperimentalDeltaProducerRecord({{SecondMeasure(), 2}, {SecondMeasure(), 3}});
  ExperimentalDeltaProducerRecord(
      {{SecondMeasure(), 4}},
      {{key1_, "value1"}, {key2_, "value2"}, {key3_, "value3"}});
  DeltaProducer::Get()->Flush();
  const opencensus::stats::ViewData data = view.GetData();
  EXPECT_THAT(
      data.double_data(),
      ::testing::UnorderedElementsAre(
          ::testing::Pair(::testing::ElementsAre("", ""), 5.0),
          ::testing::Pair(::testing::ElementsAre("value1", "value2"), 4.0)));
}

TEST_F(StatsManagerTest, IntervalDistribution) {
  ViewDescriptor view_descriptor =
      ViewDescriptor()
          .set_measure(kSecondMeasureId)
          .set_name("distribution-interval")
          .set_aggregation(
              Aggregation::Distribution(BucketBoundaries::Explicit({10})))
          .add_column(key1_)
          .add_column(key2_);
  SetAggregationWindow(AggregationWindow::Interval(absl::Hours(1)),
                       &view_descriptor);
  View view(view_descriptor);
  ASSERT_EQ(ViewData::Type::kDistribution, view.GetData().type());
  EXPECT_TRUE(view.GetData().distribution_data().empty());

  // Stats under a different measure should be ignored.
  ExperimentalDeltaProducerRecord({{FirstMeasure(), 1.0}});
  DeltaProducer::Get()->Flush();
  EXPECT_TRUE(view.GetData().distribution_data().empty());

  ExperimentalDeltaProducerRecord(
      {{SecondMeasure(), 5}, {SecondMeasure(), 15}});
  ExperimentalDeltaProducerRecord(
      {{SecondMeasure(), 5}},
      {{key1_, "value1"}, {key2_, "value2"}, {key3_, "value3"}});
  DeltaProducer::Get()->Flush();
  const opencensus::stats::ViewData data = view.GetData();
  EXPECT_EQ(2, data.distribution_data().size());
  EXPECT_EQ(std::vector<uint64_t>({1, 1}),
            data.distribution_data().find({"", ""})->second.bucket_counts());
  EXPECT_EQ(std::vector<uint64_t>({1, 0}), data.distribution_data()
                                               .find({"value1", "value2"})
                                               ->second.bucket_counts());
}

TEST_F(StatsManagerTest, IdenticalViews) {
  ViewDescriptor view_descriptor = ViewDescriptor()
                                       .set_measure(kFirstMeasureId)
                                       .set_name("count")
                                       .set_aggregation(Aggregation::Count())
                                       .add_column(key1_);

  ExperimentalDeltaProducerRecord({{FirstMeasure(), 1.0}});
  DeltaProducer::Get()->Flush();
  {
    View view1(view_descriptor);
    // No data should be recorded from before the first view is created.
    DeltaProducer::Get()->Flush();
    EXPECT_TRUE(view1.GetData().int_data().empty());
    ExperimentalDeltaProducerRecord({{FirstMeasure(), 1.0}});
    DeltaProducer::Get()->Flush();
    EXPECT_THAT(view1.GetData().int_data(),
                ::testing::UnorderedElementsAre(
                    ::testing::Pair(::testing::ElementsAre(""), 1)));
    {
      View view2(view_descriptor);
      ExperimentalDeltaProducerRecord({{FirstMeasure(), 1.0}});
      // Second views should mirror the data of the first.
      DeltaProducer::Get()->Flush();
      EXPECT_THAT(view1.GetData().int_data(),
                  ::testing::UnorderedElementsAre(
                      ::testing::Pair(::testing::ElementsAre(""), 2)));
      EXPECT_THAT(view2.GetData().int_data(),
                  ::testing::UnorderedElementsAre(
                      ::testing::Pair(::testing::ElementsAre(""), 2)));
    }
    // Removing the second view should not affect data from the first.
    DeltaProducer::Get()->Flush();
    EXPECT_THAT(view1.GetData().int_data(),
                ::testing::UnorderedElementsAre(
                    ::testing::Pair(::testing::ElementsAre(""), 2)));
  }
  // A view created after deconstructing all previous views should have data
  // reset.
  View view(view_descriptor);
  DeltaProducer::Get()->Flush();
  EXPECT_TRUE(view.GetData().int_data().empty());
}

TEST(StatsManagerDeathTest, UnregisteredMeasure) {
  const std::string measure_name = "new_measure_name";
  ViewDescriptor view_descriptor = ViewDescriptor()
                                       .set_measure(measure_name)
                                       .set_name("count")
                                       .set_aggregation(Aggregation::Count());

  View view(view_descriptor);
  EXPECT_FALSE(view.IsValid());
  DeltaProducer::Get()->Flush();
  // Getting data from an invalid view DCHECKs, and returns empty data in opt
  // mode.
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(view.GetData().int_data().empty()); }, "");
  // Even if we later register the measure and record data under it, the view
  // should still be invalid.
  static MeasureDouble measure =
      MeasureRegistry::RegisterDouble(measure_name, "", "");
  EXPECT_TRUE(measure.IsValid());
  ExperimentalDeltaProducerRecord({{measure, 1.0}});
  EXPECT_FALSE(view.IsValid());
  DeltaProducer::Get()->Flush();
  EXPECT_DEBUG_DEATH({ EXPECT_TRUE(view.GetData().int_data().empty()); }, "");
}

}  // namespace
}  // namespace stats
}  // namespace opencensus
