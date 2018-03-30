# Zipkin Export Example

This is a simple example program to demonstrate how to export tracing information
to Zipkin using OpenCensus.

Usage:

```shell
# start zipkin server
java -jar zipkin.jar --logging.level.zipkin=DEBUG --logging.level.zipkin2=DEBUG
# build OpenCensus example
bazel build :all
# run example
./bazel-bin/opencensus/exporters/trace/zipkin/zipkin_exporter_test
```

## Zipkin:

A Zipkin server needs to be running to collect the spans. Zipkin can be found [here](https://github.com/openzipkin/zipkin). Instructions on running the server can be found [here](https://github.com/openzipkin/zipkin/tree/master/zipkin-server). The Zipkin server should by default listen on port 9411. If you are running the server locally, the spans should be sent to "http://127.0.0.1:9411/api/v2/spans".

Zipkin's tracing model is not identical to the model used by OpenCensus. Here is
 a list of some of the differences when converting from OpenCensus to Zipkin.

**OpenCensus      ->  Zipkin**
  * message events  ->  annotations (serialized as <SENT/RECEIVED>/\<Id\>/\<size\>)
  * attributes        ->  tags (which are a dict of key/value pairs in Zipkin)
  * links             ->  Not Supported

## Viewing Example Trace

The traces will show up on the Zipkin webpage UI. If you are using the default address it will be "http://127.0.0.1:9411/zipkin". Click on find traces. The trace sent from the exporter should appear. Click on it and it should look something like this:

![Example Trace](https://i.imgur.com/7bNWraI.png)

The individual spans can be clicked on, which will list the tracing information
for that particular span.

![Example Span](https://i.imgur.com/S2yVHtu.png)
