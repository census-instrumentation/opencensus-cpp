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

#ifndef OPENCENSUS_PLUGINS_INTERNAL_MEASURES_H_
#define OPENCENSUS_PLUGINS_INTERNAL_MEASURES_H_

#include "opencensus/plugins/grpc/grpc_plugin.h"
#include "opencensus/stats/stats.h"

namespace opencensus {

stats::MeasureInt64 RpcClientSentMessagesPerRpc();
stats::MeasureDouble RpcClientSentBytesPerRpc();
stats::MeasureInt64 RpcClientReceivedMessagesPerRpc();
stats::MeasureDouble RpcClientReceivedBytesPerRpc();
stats::MeasureDouble RpcClientRoundtripLatency();
stats::MeasureDouble RpcClientServerLatency();
stats::MeasureInt64 RpcClientCompletedRpcs();

stats::MeasureInt64 RpcServerSentMessagesPerRpc();
stats::MeasureDouble RpcServerSentBytesPerRpc();
stats::MeasureInt64 RpcServerReceivedMessagesPerRpc();
stats::MeasureDouble RpcServerReceivedBytesPerRpc();
stats::MeasureDouble RpcServerServerLatency();
stats::MeasureInt64 RpcServerCompletedRpcs();

}  // namespace opencensus

#endif  // OPENCENSUS_PLUGINS_INTERNAL_MEASURES_H_
