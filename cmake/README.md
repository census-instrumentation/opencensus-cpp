# Experimental CMake support

The files in this directory are mostly for dealing with dependencies.

We use
[ExternalProject](https://cmake.org/cmake/help/latest/module/ExternalProject.html)
to download and build missing dependencies, during CMake's configuration time.

Still TODO for CMake support:
- Benchmarks.
- Top-level examples other than helloworld.
- Leaf examples (e.g. trace/examples/ dir).
- Exporters other than stdout.
- CI (Travis) support.
- No shared library for now.

## Quickstart:

```shell
cmake -H. -B.build
cmake --build .build
(cd .build && ctest --output-on-failure)
./.build/examples/helloworld/opencensus_examples_helloworld
```

Using `.build` as the build dir so that it doesn't collide with the uppercase
BUILD file on Windows.
