# Example HTTP server

This example uses OpenCensus to instrument an HTTP server.

Usage:

```shell
bazel run examples/http:server -- 9001
curl -i -H "traceparent: 00-404142434445464748494a4b4c4d4e4f-6162636465666768-01" 127.0.0.1:9001
```
