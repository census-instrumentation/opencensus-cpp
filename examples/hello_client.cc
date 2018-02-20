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

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

// TODO: instrument this.

#include "examples/hello.grpc.pb.h"
#include "examples/hello.pb.h"

namespace {

using examples::HelloReply;
using examples::HelloRequest;
using examples::HelloService;

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " host:port\n";
    return 1;
  }
  const std::string hostport = argv[1];

  std::shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(hostport, grpc::InsecureChannelCredentials());
  std::unique_ptr<HelloService::Stub> stub = HelloService::NewStub(channel);

  HelloRequest request;
  request.set_name("world");

  HelloReply reply;
  grpc::ClientContext ctx;

  std::cout << "Sending request: \"" << request.ShortDebugString() << "\"\n";

  grpc::Status status = stub->SayHello(&ctx, request, &reply);

  std::cout << "Got status: " << status.error_code() << ": \""
            << status.error_message() << "\"\n";
  std::cout << "Got reply: \"" << reply.ShortDebugString() << "\"\n";
}
