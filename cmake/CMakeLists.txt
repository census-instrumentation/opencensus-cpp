# ~~~
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
# ~~~

function(prepend_opencensus OUT DEPS)
  set(_DEPS "")
  foreach(dep ${DEPS})
    if("${dep}" MATCHES "::")
      list(APPEND _DEPS "${dep}")
    else()
      list(APPEND _DEPS "opencensus_${dep}")
    endif()
  endforeach()
  set(${OUT} ${_DEPS} PARENT_SCOPE)
endfunction()

# Helper function like bazel's cc_test. Usage: opencensus_test(trace_some_test
# internal/some_test.cc dep1 dep2...)
function(opencensus_test NAME SRC)
  if(BUILD_TESTING)
    set(_NAME "opencensus_${NAME}")
    add_executable(${_NAME} ${SRC})
    prepend_opencensus(DEPS "${ARGN}")
    target_link_libraries(${_NAME} "${DEPS}" gmock gtest_main)
    add_test(NAME ${_NAME} COMMAND ${_NAME})
  endif()
endfunction()

# Helper function like bazel's cc_library.  Libraries are namespaced as
# opencensus_* and public libraries are also aliased as opencensus-cpp::*.
function(opencensus_lib NAME)
  cmake_parse_arguments(ARG "PUBLIC" "" "SRCS;DEPS" ${ARGN})
  set(_NAME "opencensus_${NAME}")
  prepend_opencensus(ARG_DEPS "${ARG_DEPS}")
  if(ARG_SRCS)
    add_library(${_NAME} ${ARG_SRCS})
    target_link_libraries(${_NAME} PUBLIC ${ARG_DEPS})
    target_include_directories(${_NAME} PUBLIC ${OPENCENSUS_INCLUDE_DIR})
  else()
    add_library(${_NAME} INTERFACE)
    target_link_libraries(${_NAME} INTERFACE ${ARG_DEPS})
    target_include_directories(${_NAME} INTERFACE ${OPENCENSUS_INCLUDE_DIR})
  endif()
  if(ARG_PUBLIC)
    add_library(${PROJECT_NAME}::${NAME} ALIAS ${_NAME})
  endif()
endfunction()

# ----------------------------------------------------------------------
if(BUILD_TESTING)
  if(NOT TARGET gtest_main)
    message(STATUS "Dependency: googletest (BUILD_TESTING=${BUILD_TESTING})")

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/googletest.CMakeLists.txt
                   ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
    if(result)
      message(FATAL_ERROR "CMake step failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
                    RESULT_VARIABLE result
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
    if(result)
      message(FATAL_ERROR "Build step failed: ${result}")
    endif()

    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                     ${CMAKE_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)
  endif()
endif()

# ----------------------------------------------------------------------
# Load abseil second, it depends on googletest.
if(NOT TARGET absl::base)
  message(STATUS "Dependency: abseil")

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/abseil.CMakeLists.txt
                 ${CMAKE_BINARY_DIR}/abseil-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                  RESULT_VARIABLE result
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/abseil-download)
  if(result)
    message(FATAL_ERROR "CMake step failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
                  RESULT_VARIABLE result
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/abseil-download)
  if(result)
    message(FATAL_ERROR "Build step failed: ${result}")
  endif()

  add_subdirectory(${CMAKE_BINARY_DIR}/abseil-src
                   ${CMAKE_BINARY_DIR}/abseil-build EXCLUDE_FROM_ALL)
endif()
