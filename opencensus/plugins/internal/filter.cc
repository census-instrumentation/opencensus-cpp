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

#include "opencensus/plugins/internal/filter.h"

namespace opencensus {

void GenerateServerContext(absl::string_view tracing, absl::string_view stats,
                           absl::string_view primary_role,
                           absl::string_view method, CensusContext *context) {
  GrpcTraceContext trace_ctxt;
  TraceContextEncoding::Decode(tracing, &trace_ctxt);
  trace::SpanContext parent_ctx = trace_ctxt.ToSpanContext();
  new (context) CensusContext(method, parent_ctx);
}

void GenerateClientContext(absl::string_view method, CensusContext *context) {
  // TODO: Change this so that it can be set to root or non-root span.
  new (context) CensusContext(method);
}

size_t ServerStatsSerialize(uint64_t server_elapsed_time, char *buf,
                            size_t buf_size) {
  return RpcServerStatsEncoding::Encode(server_elapsed_time, buf, buf_size);
}

size_t ServerStatsDeserialize(const char *buf, size_t buf_size,
                              uint64_t *server_elapsed_time) {
  return RpcServerStatsEncoding::Decode(absl::string_view(buf, buf_size),
                                        server_elapsed_time);
}

uint64_t GetIncomingDataSize(const grpc_call_final_info *final_info) {
  return final_info->stats.transport_stream_stats.incoming.data_bytes;
}

uint64_t GetOutgoingDataSize(const grpc_call_final_info *final_info) {
  return final_info->stats.transport_stream_stats.outgoing.data_bytes;
}

trace::SpanContext SpanContextFromCensusContext(const census_context *ctxt) {
  return reinterpret_cast<const CensusContext *>(ctxt)->Context();
}

}  // namespace opencensus
