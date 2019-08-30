load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def opencensus_cpp_deps():
    """Loads dependencies need to compile the opencensus-cpp library."""

    maybe(
        http_archive,
        name = "rules_cc",
        strip_prefix = "rules_cc-master",
        urls = ["https://github.com/bazelbuild/rules_cc/archive/master.zip"],
    )

    # We depend on Abseil.
    maybe(
        http_archive,
        name = "com_google_absl",
        strip_prefix = "abseil-cpp-master",
        urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
    )

    # gRPC
    maybe(
        http_archive,
        name = "com_github_grpc_grpc",
        strip_prefix = "grpc-master",
        urls = ["https://github.com/grpc/grpc/archive/master.tar.gz"],
    )

    # Prometheus client library - used by Prometheus exporter.
    maybe(
        http_archive,
        name = "com_github_jupp0r_prometheus_cpp",
        strip_prefix = "prometheus-cpp-master",
        urls = ["https://github.com/jupp0r/prometheus-cpp/archive/master.zip"],
    )

    # Curl library - used by zipkin exporter.
    maybe(
        http_archive,
        name = "com_github_curl",
        build_file = "//:bazel/curl.BUILD",
        strip_prefix = "curl-master",
        urls = ["https://github.com/curl/curl/archive/master.zip"],
    )

    # Rapidjson library - used by zipkin exporter.
    maybe(
        http_archive,
        name = "com_github_tencent_rapidjson",
        build_file = "//:bazel/rapidjson.BUILD",
        strip_prefix = "rapidjson-master",
        urls = ["https://github.com/Tencent/rapidjson/archive/master.zip"],
    )

    # Google APIs - used by Stackdriver exporter.
    maybe(
        http_archive,
        name = "com_google_googleapis",
        strip_prefix = "googleapis-master",
        urls = ["https://github.com/googleapis/googleapis/archive/master.zip"],
    )

    # Needed by opencensus-proto.
    maybe(
        http_archive,
        name = "io_bazel_rules_go",
        sha256 = "9fb16af4d4836c8222142e54c9efa0bb5fc562ffc893ce2abeac3e25daead144",
        urls = [
            "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/rules_go/releases/download/0.19.0/rules_go-0.19.0.tar.gz",
            "https://github.com/bazelbuild/rules_go/releases/download/0.19.0/rules_go-0.19.0.tar.gz",
        ],
    )

    # OpenCensus proto - used by OcAgent exporter.
    maybe(
        http_archive,
        name = "opencensus_proto",
        strip_prefix = "opencensus-proto-master/src",
        urls = ["https://github.com/census-instrumentation/opencensus-proto/archive/master.zip"],
    )
