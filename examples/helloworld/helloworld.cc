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

#include <time.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "opencensus/exporters/stats/stdout/stdout_exporter.h"
#include "opencensus/exporters/trace/stdout/stdout_exporter.h"
#include "opencensus/stats/stats.h"
#include "opencensus/trace/sampler.h"
#include "opencensus/trace/span.h"

namespace {

ABSL_CONST_INIT const absl::string_view kVideoSizeViewName =
    "my.org/views/video_size";
ABSL_CONST_INIT const absl::string_view kVideoSizeMeasureName =
    "my.org/measure/video_size";

// The resource owner defines and registers a measure. A function exposing a
// function-local static is the recommended style, ensuring that the measure is
// only registered once.
opencensus::stats::MeasureInt64 VideoSizeMeasure() {
  static const opencensus::stats::MeasureInt64 video_size =
      opencensus::stats::MeasureInt64::Register(
          kVideoSizeMeasureName, "size of processed videos", "By");
  return video_size;
}

opencensus::tags::TagKey FrontendKey() {
  static const auto frontend_key =
      opencensus::tags::TagKey::Register("my.org/keys/frontend");
  return frontend_key;
}

}  // namespace

// Simple program that collects data for video size.
int main(int argc, char **argv) {
  if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << "\n";
    return 1;
  }
  srand(time(NULL));

  // Register stdout exporters.
  opencensus::exporters::stats::StdoutExporter::Register();
  opencensus::exporters::trace::StdoutExporter::Register();

  // Call measure so that it is initialized.
  VideoSizeMeasure();

  constexpr int64_t kMiB = 1 << 20;

  // Create view to see the processed video size distribution broken down by
  // frontend. The view has bucket boundaries (0, 16 * kMiB, 65536 * kMiB) that
  // will group measure values into histogram buckets.
  const opencensus::stats::ViewDescriptor video_size_view =
      opencensus::stats::ViewDescriptor()
          .set_name(kVideoSizeViewName)
          .set_description("processed video size over time")
          .set_measure(kVideoSizeMeasureName)
          .set_aggregation(opencensus::stats::Aggregation::Distribution(
              opencensus::stats::BucketBoundaries::Explicit(
                  {0, 16 * kMiB, 256 * kMiB})))
          .add_column(FrontendKey());
  opencensus::stats::View view(video_size_view);
  video_size_view.RegisterForExport();

  // Samplers are potentially expensive to construct. Use one long-lived sampler
  // instead of constructing one per Span.
  static opencensus::trace::AlwaysSampler sampler;

  // Done initializing. Video processing starts here:
  auto span = opencensus::trace::Span::StartSpan("my.org/ProcessVideo", nullptr,
                                                 {&sampler});
  span.AddAnnotation("Start processing video.");
  // Sleep for [1,10] milliseconds to fake work.
  absl::SleepFor(absl::Milliseconds(rand() % 10 + 1));
  // Record the processed video size.
  opencensus::stats::Record({{VideoSizeMeasure(), 25 * kMiB}},
                            {{FrontendKey(), "video size"}});
  span.AddAnnotation("Finished processing video.");
  span.End();

  std::cout << "video_size_view definitions:" << video_size_view.DebugString()
            << "\n\n";

  std::cout << "Waiting 10.1s for exporters to run...\n\n";
  absl::SleepFor(absl::Milliseconds(10100));

  std::cout << "View data:\n";
  const auto data = view.GetData();
  assert(data.type() == opencensus::stats::ViewData::Type::kDistribution);
  for (auto &it : data.distribution_data()) {
    std::cout << "  ";
    for (auto &name : it.first) std::cout << name << " : ";
    std::cout << it.second.DebugString() << "\n";
  }
}
