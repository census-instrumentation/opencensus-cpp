# Example RPC server

This example uses:

* gRPC to create an RPC server and client.
* The OpenCensus gRPC plugin to instrument the RPC server.
* The Stackdriver exporter to export stats and traces.
* The Prometheus exporter to export stats.
* Debugging exporters to print stats and traces to stdout.

Usage:

```shell
bazel build :all
../../bazel-bin/examples/grpc/hello_server 9001
../../bazel-bin/examples/grpc/hello_client [::]:9001
```

You can see the Prometheus stats on http://127.0.0.1:8080/metrics

To increase gRPC verbosity, try:

```shell
env GRPC_VERBOSITY=DEBUG ../../bazel-bin/examples/grpc/hello_server 9001
env GRPC_VERBOSITY=DEBUG ../../bazel-bin/examples/grpc/hello_client [::]:9001
```

## Running the example on GCE

Log in to a GCE instance:

```shell
gcloud compute --project myproject ssh myinstance
```

Install the build tools:

```shell
sudo apt install pkg-config zip g++ zlib1g-dev unzip python
wget https://github.com/bazelbuild/bazel/releases/download/0.11.0/bazel-0.11.0-installer-linux-x86_64.sh
chmod +x bazel-0.11.0-installer-linux-x86_64.sh
./bazel-0.11.0-installer-linux-x86_64.sh --user
```

Build the example:

```shell
git clone https://github.com/census-instrumentation/opencensus-cpp.git
cd opencensus-cpp/examples/grpc
bazel build :all
```

Install the gRPC certificates on the VM: (you only need to do this once)

```shell
sudo mkdir -p /usr/share/grpc
sudo cp $(bazel info output_base)/external/com_github_grpc_grpc/etc/roots.pem /usr/share/grpc/roots.pem
```

Run the server but set `STACKDRIVER_PROJECT_ID`, e.g.:

```shell
env STACKDRIVER_PROJECT_ID=myproject ../../bazel-bin/examples/grpc/hello_server 9001
env STACKDRIVER_PROJECT_ID=myproject ../../bazel-bin/examples/grpc/hello_client [::]:9001
```

### Stats

Go to https://console.cloud.google.com/monitoring to see stats:

* Click on Resources &rarr; Metrics Explorer.
* Choose the "global" Resource Type.

### Tracing

Go to https://console.cloud.google.com/traces/traces to see traces. Example:

![Example trace](img/sample_trace.png)

When running on GCE, `grpc::GoogleDefaultCredentials()` works without having to
generate a key with the `gcloud` CLI.
