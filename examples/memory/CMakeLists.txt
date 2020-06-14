if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

  include( FetchContent )

  FetchContent_Declare(
    re2
    URL https://github.com/google/re2/archive/2020-06-01.tar.gz
    URL_HASH    SHA256=fb8e0f4ed7a212e3420507f27933ef5a8c01aec70e5148c6a35313573269fae6
    )

  #find_package( absl REQUIRED )

  FetchContent_GetProperties(re2)
  if(NOT re2_POPULATED)
    # build re2 as as a private static library with ashared
    set(orig_BUILD_SHARED_LIBS          "${BUILD_SHARED_LIBS}"         )
    set( orig_POSITION_INDEPENDENT_CODE "${POSITION_INDEPENDENT_CODE}" )

    set( BUILD_SHARED_LIBS         OFF )
    set( POSITION_INDEPENDENT_CODE YES )

    FetchContent_Populate(re2)
    add_subdirectory(${re2_SOURCE_DIR} ${re2_BINARY_DIR} EXCLUDE_FROM_ALL )

    set( BUILD_SHARED_LIBS          "${orig_BUILD_SHARED_LIB}"          )
    set ( POSITION_INDEPENDENT_CODE "${orig_POSITION_INDEPENDENT_CODE}" )

  endif()

  set ( USE_SYSTEM_CURL YES )
  include(FetchContent)
  FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git)
  if ( NOT cpr_POPULATED )

    set(orig_BUILD_SHARED_LIBS          "${BUILD_SHARED_LIBS}"         )
    set( orig_POSITION_INDEPENDENT_CODE "${POSITION_INDEPENDENT_CODE}" )

    set( BUILD_SHARED_LIBS         OFF )
    set( POSITION_INDEPENDENT_CODE YES )

    FetchContent_Populate(cpr)
    add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR} EXCLUDE_FROM_ALL )

    set( BUILD_SHARED_LIBS          "${orig_BUILD_SHARED_LIB}"          )
    set ( POSITION_INDEPENDENT_CODE "${orig_POSITION_INDEPENDENT_CODE}" )

  endif()


  add_executable(linux_memory_stats_to_stackdriver memory_stats.cc )
  target_include_directories( linux_memory_stats_to_stackdriver PRIVATE "${re2_SOURCE_DIR}" )
  target_link_libraries(
    linux_memory_stats_to_stackdriver
    absl::flags
    absl::flags_parse
    absl::strings
    absl::time
    opencensus-cpp::exporters_stats_stackdriver
    opencensus-cpp::exporters_stats_stdout
    opencensus-cpp::stats
    re2
    cpr::cpr
    )

endif()
