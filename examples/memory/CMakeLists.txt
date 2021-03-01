if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

  include(FetchContent)

  # find_package( absl REQUIRED )

  if(BUILD_STACKDRIVER_EXPORTER)

    set(USE_SYSTEM_CURL YES)
    include(FetchContent)
    FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git)
    if(NOT cpr_POPULATED)

      set(orig_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")
      set(orig_POSITION_INDEPENDENT_CODE "${POSITION_INDEPENDENT_CODE}")

      set(BUILD_SHARED_LIBS OFF)
      set(POSITION_INDEPENDENT_CODE YES)

      FetchContent_Populate(cpr)
      add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR} EXCLUDE_FROM_ALL)

      set(BUILD_SHARED_LIBS "${orig_BUILD_SHARED_LIB}")
      set(POSITION_INDEPENDENT_CODE "${orig_POSITION_INDEPENDENT_CODE}")

    endif()

    add_executable(linux_memory_stats_to_stackdriver memory_stats.cc)
    target_include_directories(linux_memory_stats_to_stackdriver
                               PRIVATE "${re2_SOURCE_DIR}")
    target_compile_definitions(linux_memory_stats_to_stackdriver
                               PRIVATE USE_STACKDRIVER_EXPORTER)

    target_link_libraries(
      linux_memory_stats_to_stackdriver
      absl::flags
      absl::flags_parse
      absl::strings
      absl::time
      opencensus-cpp::exporters_stats_stackdriver
      opencensus-cpp::exporters_stats_stdout
      opencensus-cpp::stats
      cpr::cpr)

  endif(BUILD_STACKDRIVER_EXPORTER)

  add_executable(linux_memory_stats_to_stdout memory_stats.cc)
  target_include_directories(linux_memory_stats_to_stdout
                             PRIVATE "${re2_SOURCE_DIR}")
  target_link_libraries(
    linux_memory_stats_to_stdout
    absl::flags
    absl::flags_parse
    absl::strings
    absl::time
    opencensus-cpp::exporters_stats_stdout
    opencensus-cpp::stats)

endif() # Linux
