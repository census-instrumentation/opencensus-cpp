# Example RPC server

This example uses the gRPC plugin to instrument its RPC server, and the
Stackdriver exporter to export stats and traces.

Usage:
```shell
bazel build :all
env GRPC_VERBOSITY=DEBUG STACKDRIVER_PROJECT_ID=e2e-debugging ../bazel-bin/examples/hello_server 9001
env GRPC_VERBOSITY=DEBUG ../bazel-bin/examples/hello_client [::]:9001
```

You can see the Prometheus stats on [http://127.0.0.1:8080].

TODO: more and better docs

TODO: instrument the client also
