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
../../bazel-bin/examples/grpc/hello_client "[::]:9001"
```

You can see the Prometheus stats on http://127.0.0.1:8080/metrics

To increase gRPC verbosity, try:

```shell
env GRPC_VERBOSITY=DEBUG ../../bazel-bin/examples/grpc/hello_server 9001
env GRPC_VERBOSITY=DEBUG ../../bazel-bin/examples/grpc/hello_client "[::]:9001"
```

## Stackdriver

In order to be able to push your stats to [Stackdriver Monitoring](https://cloud.google.com/monitoring/), you must:

1. [Create a Cloud project](https://support.google.com/cloud/answer/6251787?hl=en).
1. [Enable billing](https://support.google.com/cloud/answer/6288653#new-billing).
1. [Enable the Stackdriver Monitoring API](https://app.google.stackdriver.com/).
1. [Make sure you have a Premium Stackdiver account](https://cloud.google.com/monitoring/accounts/tiers).

To use our Stackdriver Stats exporter, your Stackdriver account needs to have
permission to [create custom metrics](https://cloud.google.com/monitoring/custom-metrics/creating-metrics),
and that requires a [Premium tier Stackdriver account](https://cloud.google.com/monitoring/accounts/tiers#this_request_is_only_available_in_the_premium_tier).
Please note that by default all new Stackdriver accounts are Basic tier. To
upgrade to a Premium tier Stackdriver account, follow the instructions
[here](https://cloud.google.com/monitoring/accounts/tiers#start-premium).

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

When running on GCE, the credentials used to authenticate to Stackdriver are
determined automatically from the VM.

### Stats

Go to https://console.cloud.google.com/monitoring to see stats:

* Click on Resources &rarr; Metrics Explorer.
* Choose the "global" Resource Type.

Example:

![Example of stats](https://i.imgur.com/OSomV4W.png)

### Tracing

Go to https://console.cloud.google.com/traces/traces to see traces. Example:

![Example trace](https://i.imgur.com/iYPCZOr.png)
