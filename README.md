> **Warning**
>
> OpenCensus and OpenTracing have merged to form [OpenTelemetry](https://opentelemetry.io), which serves as the next major version of OpenCensus and OpenTracing.
>
> OpenTelemetry has now reached feature parity with OpenCensus, with tracing and metrics SDKs available in .NET, Golang, Java, NodeJS, and Python. **All OpenCensus Github repositories, except [census-instrumentation/opencensus-python](https://github.com/census-instrumentation/opencensus-python), will be archived on July 31st, 2023**. We encourage users to migrate to OpenTelemetry by this date.
>
> To help you gradually migrate your instrumentation to OpenTelemetry, bridges are available in Java, Go, Python, and JS. [**Read the full blog post to learn more**](https://opentelemetry.io/blog/2023/sunsetting-opencensus/).

# OpenCensus - A stats collection and distributed tracing framework
[![Gitter chat][gitter-image]][gitter-url]
[![Travis Build Status][travis-image]][travis-url]
[![Appveyor Build Status][appveyor-image]][appveyor-url]

OpenCensus is a toolkit for collecting application performance and behavior data. It currently
includes an API for tracing and stats.

This library is currently in alpha: the API is in the process of being
finalized; much of the implementation will be replaced with a more optimized
one in the near future.

Please join [gitter](https://gitter.im/census-instrumentation/Lobby) for help or feedback on this
project.

This is not an officially supported Google product.

## Quickstart

Please refer to [`examples/helloworld/`](examples/helloworld) for an example of
instrumentation with OpenCensus.

Please refer to [`examples/grpc/`](examples/grpc) for an example RPC
server that integrates gRPC, Stackdriver, and Prometheus.

Please refer to
[`trace/examples/span_example.cc`](opencensus/trace/examples/span_example.cc)
for tracing and
[`stats/examples/view_and_recording_example.cc`](opencensus/stats/examples/view_and_recording_example.cc)
for stats.

## Directory structure

* [`opencensus/`](opencensus) prefix to get `#include` paths like `opencensus/trace/span.h`
  * [`common/`](opencensus/common) - Provides common libraries and components for OpenCensus.
  * [`doc/`](opencensus/doc) - Documentation for our APIs, coding style, etc.
  * [`exporters/`](opencensus/exporters) - Exporters send stats and traces to
    monitoring services.
  * [`plugins/`](opencensus/plugins) - Plugins add OpenCensus instrumentation to
    frameworks.
  * [`stats/`](opencensus/stats) - OpenCensus stats API.
  * [`trace/`](opencensus/trace) - OpenCensus tracing API.

## Language support

* STL is required. (e.g. `libstdc++`, `libc++`, etc)
* C++14 is required.
* [`absl`](https://github.com/abseil/abseil-cpp/) is used for its building blocks.
* [`googletest`](https://github.com/google/googletest/) is used for tests.
* [`benchmark`](https://github.com/google/benchmark/) is used for benchmarking.
* We do not depend on:
  * Boost
  * Exception handling
  * RTTI

## Compiler support

We are targeting the following compilers:

* gcc 4.8.1
* clang 3.4
* Microsoft Visual Studio 2017 15.9 (see [the appveyor documentation](https://www.appveyor.com/docs/windows-images-software/#visual-studio-2017) for the exact version)

[gitter-image]: https://badges.gitter.im/census-instrumentation/lobby.svg
[gitter-url]: https://gitter.im/census-instrumentation/lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge
[travis-image]: https://travis-ci.org/census-instrumentation/opencensus-cpp.svg?branch=master
[travis-url]: https://travis-ci.org/census-instrumentation/opencensus-cpp
[appveyor-image]: https://ci.appveyor.com/api/projects/status/github/census-instrumentation/opencensus-cpp?branch=master&svg=true
[appveyor-url]: https://ci.appveyor.com/project/opencensuscppteam/opencensus-cpp
