find_package( gRPC CONFIG REQUIRED)

opencensus_lib(common_grpc_status
              HDRS
	      status.h
              SRCS
              status.cc
              DEPS
              absl::strings
)
get_target_property( _iface_incdir gRPC::grpc++ INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories( opencensus_common_grpc_status PUBLIC $<BUILD_INTERFACE:${_iface_incdir}> )
target_link_libraries     ( opencensus_common_grpc_status PUBLIC gRPC::grpc++ )

opencensus_test(common_grpc_status_test
                status_test.cc
                common_grpc_status )

opencensus_lib(common_grpc_with_user_agent
              HDRS
	      with_user_agent.h
              SRCS
              with_user_agent.cc
              DEPS
              absl::strings
)
target_include_directories( opencensus_common_grpc_with_user_agent PUBLIC $<BUILD_INTERFACE:${_iface_incdir}> )
target_link_libraries     ( opencensus_common_grpc_with_user_agent PUBLIC gRPC::grpc++ )
