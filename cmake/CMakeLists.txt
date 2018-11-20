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

cmake_minimum_required(VERSION 3.5)

# ----------------------------------------------------------------------
if(BUILD_TESTING)
  if(NOT TARGET gtest_main)
    message(STATUS "Dependency: googletest")

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

# Helper function like bazel's cc_test. Usage:
# opencensus_test(dir_name.test_name internal/test_source.cc dep1 dep2)
function(opencensus_test NAME SRC)
  if(BUILD_TESTING)
    add_executable(${NAME} ${SRC})
    target_link_libraries(${NAME} ${ARGN} gtest_main)
    add_test(NAME ${NAME} COMMAND ${NAME})
  endif()
endfunction()

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
