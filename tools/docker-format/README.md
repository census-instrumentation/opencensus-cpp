Docker container for automatic formatting.

Usage:

```shell
cd opencensus-cpp
docker build --tag opencensus-cpp/format tools/docker-format
docker run -v $PWD:/opencensus-cpp -it opencensus-cpp/format /opencensus-cpp/tools/docker-format/run.sh
```
