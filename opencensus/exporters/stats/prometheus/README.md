# OpenCensus Prometheus Stats Exporter

The OpenCensus Prometheus Stats Exporter is a stats exporter that exposes stats
to the
[Prometheus cpp client library](https://github.com/jupp0r/prometheus-cpp).

## Quickstart

### Prerequisites

The Prometheus exporter exposes stats on an HTTP endpoint that is scraped by
a Prometheus server. To install and run the Prometheus server, follow
the instructions [here](https://prometheus.io/docs/introduction/first_steps/).

### Register the exporter

#### Using the Prometheus client library's exposer

Register the Exposer on a selected port, and then register the OpenCensus
Prometheus exporter with that exposer. This may be done either before or after
views are registered for export and data is recorded, but stats will not be
available to Prometheus until the exporter is registered.

```c++
  auto exporter =
      std::make_shared<opencensus::exporters::stats::PrometheusExporter>();

  // Expose a Prometheus endpoint.
  prometheus::Exposer exposer("127.0.0.1:8080");
  exposer.RegisterCollectable(exporter);
```

See also the Prometheus client library's
[instructions](https://github.com/jupp0r/prometheus-cpp#usage).

#### Using a custom exposer

If your application already runs an HTTP server, you may want to expose
Prometheus stats on a page by that server rather than through the default
exposer. To do this, create an instance of the `PrometheusExporter` and then
call `Collect()` to retrieve the latest stats, which can be serialized with the
tools in the Prometheus client library:

```c++
#include "metrics.pb.h"
#include "opencensus/exporters/stats/prometheus/prometheus_exporter.h"
#include "prometheus/text_serializer.h"

...

opencensus::exporters::stats::PrometheusExporter exporter;
prometheus::TextSerializer serializer;

const std::vector<io::prometheus::client::MetricFamily> metrics =
    exporter.Collect();
const std::string formatted_metrics = serializer.Serialize(metrics);

```
