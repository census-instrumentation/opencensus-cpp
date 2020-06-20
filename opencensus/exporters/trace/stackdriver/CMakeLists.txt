opencensus_lib( exporters_trace_stackdriver
                PUBLIC
		HDRS
		stackdriver_exporter.h
                SRCS
                internal/stackdriver_exporter.cc
                DEPS
                common_hostname
                common_grpc_status
                common_grpc_with_user_agent
                trace
                )

find_package( googleapis CONFIG REQUIRED googleapis_cpp_devtools_cloudtrace_v2_protos )

#get_target_property(_iface_incdir googleapis_cpp_devtools_cloudtrace_v2_protos INTERFACE_INCLUDE_DIRECTORIES )
#
#target_include_directories( opencensus_exporters_trace_stackdriver PRIVATE ${_iface_incdir})
target_link_libraries( opencensus_exporters_trace_stackdriver
                       PUBLIC
		       googleapis_cpp_devtools_cloudtrace_v2_trace_protos
		       googleapis_cpp_devtools_cloudtrace_v2_tracing_protos
		       googleapis_cpp_rpc_status_protos )

