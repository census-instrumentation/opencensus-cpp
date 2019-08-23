load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def opencensus_cpp_deps():
    """Loads dependencies need to compile and test the opencensus-cpp library."""

    # native.bind():

    if "com_google_protobuf" not in native.existing_rules():
        http_archive(
            name = "com_google_protobuf",
            sha256 = "416212e14481cff8fd4849b1c1c1200a7f34808a54377e22d7447efdf54ad758",
            strip_prefix = "protobuf-09745575a923640154bcf307fba8aedff47f240a",
            url = "https://github.com/google/protobuf/archive/09745575a923640154bcf307fba8aedff47f240a.tar.gz",
        )

    if "rules_cc" not in native.existing_rules():
        # Build rules for C++ projects.
        http_archive(
            name = "rules_cc",
            strip_prefix = "rules_cc-master",
            urls = ["https://github.com/bazelbuild/rules_cc/archive/master.zip"],
        )

    if "com_google_absl" not in native.existing_rules():
        # We depend on Abseil.
        http_archive(
            name = "com_google_absl",
            strip_prefix = "abseil-cpp-master",
            urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
        )

    if "com_google_googletest" not in native.existing_rules():
        # GoogleTest framework.
        # Only needed for tests, not to build the OpenCensus library.
        http_archive(
            name = "com_google_googletest",
            strip_prefix = "googletest-master",
            urls = ["https://github.com/google/googletest/archive/master.zip"],
        )

    if "com_github_google_benchmark" not in native.existing_rules():
        # Google Benchmark library.
        # Only needed for benchmarks, not to build the OpenCensus library.
        http_archive(
            name = "com_github_google_benchmark",
            strip_prefix = "benchmark-master",
            urls = ["https://github.com/google/benchmark/archive/master.zip"],
        )

    if "build_bazel_rules_apple" not in native.existing_rules():
        # Used by gRPC.
        http_archive(
            name = "build_bazel_rules_apple",
            strip_prefix = "rules_apple-master",
            urls = ["https://github.com/bazelbuild/rules_apple/archive/master.zip"],
        )

    if "build_bazel_apple_support" not in native.existing_rules():
        # Used by gRPC.
        http_archive(
            name = "build_bazel_apple_support",
            strip_prefix = "apple_support-master",
            urls = ["https://github.com/bazelbuild/apple_support/archive/master.zip"],
        )

    if "com_github_grpc_grpc" not in native.existing_rules():
        # gRPC
        http_archive(
            name = "com_github_grpc_grpc",
            strip_prefix = "grpc-master",
            urls = ["https://github.com/grpc/grpc/archive/master.tar.gz"],
        )

    if "com_github_jupp0r_prometheus_cpp" not in native.existing_rules():
        # Prometheus client library - used by Prometheus exporter.
        http_archive(
            name = "com_github_jupp0r_prometheus_cpp",
            strip_prefix = "prometheus-cpp-master",
            urls = ["https://github.com/jupp0r/prometheus-cpp/archive/master.zip"],
        )

    if "com_github_curl" not in native.existing_rules():
        # Curl library - used by zipkin exporter.
        http_archive(
            name = "com_github_curl",
            build_file_content =
                """
        load("@io_opencensus_cpp//opencensus:curl.bzl", "CURL_COPTS")
        package(features = ['no_copts_tokenization'])
        
        config_setting(
            name = "windows",
            values = {"cpu": "x64_windows"},
            visibility = [ "//visibility:private" ],
        )
        
        config_setting(
            name = "osx",
            values = {"cpu": "darwin"},
            visibility = [ "//visibility:private" ],
        )
        
        cc_library(
            name = "curl",
            srcs = glob([
                "lib/**/*.c",
            ]),
            hdrs = glob([
                "include/curl/*.h",
                "lib/**/*.h",
            ]),
            includes = ["include/", "lib/"],
            copts = CURL_COPTS + [
                "-DOS=\\"os\\"",
                "-DCURL_EXTERN_SYMBOL=__attribute__((__visibility__(\\"default\\")))",
            ],
            visibility = ["//visibility:public"],
        )
        """,
            strip_prefix = "curl-master",
            urls = ["https://github.com/curl/curl/archive/master.zip"],
        )

    if "com_github_tencent_rapidjson" not in native.existing_rules():
        # Rapidjson library - used by zipkin exporter.
        http_archive(
            name = "com_github_tencent_rapidjson",
            build_file_content =
                """
        cc_library(
            name = "rapidjson",
            srcs = [],
            hdrs = glob([
                "include/rapidjson/*.h",
                "include/rapidjson/internal/*.h",
                "include/rapidjson/error/*.h",
            ]),
            includes = ["include/"],
            visibility = ["//visibility:public"],
        )
        """,
            strip_prefix = "rapidjson-master",
            urls = ["https://github.com/Tencent/rapidjson/archive/master.zip"],
        )

    if "com_google_googleapis" not in native.existing_rules():
        # Google APIs - used by Stackdriver exporter.
        http_archive(
            name = "com_google_googleapis",
            strip_prefix = "googleapis-master",
            urls = ["https://github.com/googleapis/googleapis/archive/master.zip"],
        )

    if "io_bazel_rules_go" not in native.existing_rules():
        # Needed by opencensus-proto.
        http_archive(
            name = "io_bazel_rules_go",
            sha256 = "9fb16af4d4836c8222142e54c9efa0bb5fc562ffc893ce2abeac3e25daead144",
            urls = [
                "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/rules_go/releases/download/0.19.0/rules_go-0.19.0.tar.gz",
                "https://github.com/bazelbuild/rules_go/releases/download/0.19.0/rules_go-0.19.0.tar.gz",
            ],
        )

    if "grpc_java" not in native.existing_rules():
        # Needed by opencensus-proto.
        http_archive(
            name = "grpc_java",
            strip_prefix = "grpc-java-master",
            urls = ["https://github.com/grpc/grpc-java/archive/master.zip"],
        )

    if "io_opencensus_proto" not in native.existing_rules():
        # OpenCensus proto - used by OcAgent exporter.
        http_archive(
            name = "io_opencensus_proto",
            strip_prefix = "opencensus-proto-master/src",
            urls = ["https://github.com/census-instrumentation/opencensus-proto/archive/master.zip"],
        )
