# OpenCensus Stackdriver Stats Exporter

The *OpenCensus Stackdriver Stats Exporter* is a stats exporter that exports
data to [Stackdriver Monitoring](stackdriver-monitoring).

## Quickstart

### Prerequisites

The Stackdriver exporter can be used from any internet-connected application,
whether on Google Cloud Platform, another cloud platform, or on-premise.

In order to be able to push your stats to [Stackdriver Monitoring](stackdriver-monitoring), you must:
1. [Create a Cloud project](https://support.google.com/cloud/answer/6251787?hl=en).
2. [Enable billing](https://support.google.com/cloud/answer/6288653#new-billing).
3. [Enable the Stackdriver Monitoring API](https://app.google.stackdriver.com/).
4. [Make sure you have a Premium Stackdriver account](https://cloud.google.com/monitoring/accounts/tiers).

These steps enable the API but don't require that your app is hosted on Google Cloud Platform.

### Setup authentication

The Stackdriver exporter uses gRPC, which requires a certificate
(`etc/roots.pem` in the gRPC repository) copied to
to `/usr/share/grpc/roots.pem`.

If your application runs on Google Cloud Platform, it can automatically
determine credentials to authenticate to Stackdriver from the VM environment.

Otherwise, create a
[Google Cloud service account](https://cloud.google.com/iam/docs/creating-managing-service-accounts),
create and download a service account key,
and set the environment variable `GOOGLE_APPLICATION_CREDENTIALS` to the path to
that key.

Please refer to the [Stats access controls](https://cloud.google.com/monitoring/access-control)
and [Trace access controls](https://cloud.google.com/trace/docs/iam)
documentation for configuring roles. The "Monitoring Editor" role is required.

### Register the exporter

Include:

```c++
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
```

Add a BUILD dependency on:

```
"@io_opencensus_cpp//exporters/stats/stackdriver:stackdriver_exporter",
```

In your application's initialization code, register the exporter:

```c++
opencensus::exporters::stats::StackdriverOptions opts;
opts.project_id = "my-stackdriver-project-id";
opts.opencensus_task = absl::StrCat("cpp-", getpid(), "@", hostname);

opencensus::exporters::stats::StackdriverExporter::Register(opts);
```

The `opencensus_task` may be anything, but must be unique among all processes
simultaneously exporting to Stackdriver. The format
`"cpp-${PROCESS_ID}@${HOSTNAME}"` is recommended.

### Register views and record stats

Once the exporter has been registered, any stats for views registered with
`ViewDescriptor::RegisterForExport()` will be exported. Views may be registered
before or after the exporter, and will track stats since the registration of the
view in either case.

## Data model

### View metadata

OpenCensus exports views as custom metrics under the
[Global](https://cloud.google.com/monitoring/api/resources#tag_global)
monitored resource. For each view, OpenCensus creates a Stackdriver metric. This
metric's name will be under the path `"custom.googleapis.com/opencensus/"`, so
that, for example, a view named `"example.com/client/latency"` would translate
to a Stackdriver metric named
`"custom.googleapis.com/opencensus/example.com/client/latency"`.

Views with `LastValue` aggregation will be registered as `GAUGE` Stackdriver
metrics, and other aggregations will be registered as `CUMULATIVE` metrics.

View columns translate to Stackdriver's labels. OpenCensus adds a label
`"opencensus_task"` to all exported views so that exports from different
processes do not conflict.

### Data

For each row of the view's data (a unique combination of tag values) OpenCensus
exports a separate `TimeSeries` with label values corresponding to the row's tag
values.

The Stackdriver data type depends on the
view's measure type and aggregation--count aggregation to `INT64`, sum
aggregation to `INT64` for `MeasureInt64` and `DOUBLE` for `MeasureDouble`, and
distribution aggregation to `DISTRIBUTION`. Exported distributions omit the
range as it is not supported by Stackdriver.
