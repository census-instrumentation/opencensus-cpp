# Example HTTP server

This example uses OpenCensus to instrument an HTTP server.

Usage:

```shell
bazel run examples/http:server -- 9001
curl -i -H "traceparent: 00-404142434445464748494a4b4c4d4e4f-6162636465666768-01" 127.0.0.1:9001
```

Prometheus metrics can be seen at http://127.0.0.1:8080/metrics
and running Zipkin will show traces, e.g.:

```shell
docker run -p 9411:9411 openzipkin/zipkin
```
