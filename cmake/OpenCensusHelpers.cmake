# Copyright 2018, OpenCensus Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Helper function like bazel's cc_test. Usage:
#
# opencensus_test(trace_some_test internal/some_test.cc dep1 dep2...)
function(opencensus_test NAME SRC)
  if(BUILD_TESTING AND OpenCensus_BUILD_TESTING)
    add_executable(${NAME} ${SRC})
    target_include_directories(${NAME}
      PUBLIC
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    target_link_libraries(${NAME} "${ARGN}" gmock gtest_main)
    add_test(NAME ${NAME} COMMAND ${NAME})
  endif()
endfunction()

# Helper function like bazel's cc_benchmark. Usage:
#
# opencensus_benchmark(trace_some_benchmark internal/some_benchmark.cc dep1
# dep2...)
function(opencensus_benchmark NAME SRC)
  if(BUILD_TESTING AND OpenCensus_BUILD_TESTING)
    add_executable(${NAME} ${SRC})
    target_include_directories(${NAME}
      PUBLIC
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    target_link_libraries(${NAME} "${ARGN}" benchmark)
  endif()
endfunction()

# Helper function like bazel's cc_library.  Libraries are namespaced as
# opencensus_* and public libraries are also aliased as opencensus-cpp::*.
function(opencensus_lib NAME)
  cmake_parse_arguments(ARG "PUBLIC" "" "SRCS;DEPS" ${ARGN})
  if(ARG_SRCS)
    add_library(${NAME} ${ARG_SRCS})
    target_link_libraries(${NAME} PUBLIC ${ARG_DEPS})
    target_include_directories(${NAME}
      PUBLIC
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  else()
    add_library(${NAME} INTERFACE)
    target_link_libraries(${NAME} INTERFACE ${ARG_DEPS})
    target_include_directories(${NAME}
      INTERFACE
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  endif()
  install(TARGETS ${NAME} EXPORT OpenCensusTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
  if(ARG_PUBLIC)
    add_library(${PROJECT_NAME}::${NAME} ALIAS ${NAME})
  endif()
endfunction()

# Helper function for fuzzing. Usage:
#
# opencensus_fuzzer(trace_some_fuzzer internal/some_fuzzer.cc dep1 dep2...)
function(opencensus_fuzzer NAME SRC)
  if(FUZZER)
    add_executable(${NAME} ${SRC})
    target_link_libraries(${NAME} "${DEPS}" ${FUZZER})
    target_compile_options(${NAME} PRIVATE ${FUZZER})
    target_include_directories(${NAME}
      PUBLIC
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  endif()
endfunction()
