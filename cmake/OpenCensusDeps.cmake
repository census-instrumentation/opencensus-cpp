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

if(BUILD_TESTING)
  if(NOT TARGET gtest_main)
    message(STATUS "Dependency: googletest (BUILD_TESTING=${BUILD_TESTING})")
    
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      # force this option to ON so that Google Test will use /MD instead of /MT
      # /MD is now the default for Visual Studio, so it should be our default, too
      option(gtest_force_shared_crt
        "Use shared (DLL) run-time lib even when Google Test is built as static lib."
        ON)
    endif()
    
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/googletest.CMakeLists.txt
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

# Load abseil second, it depends on googletest.
if(NOT TARGET absl::base)
  message(STATUS "Dependency: abseil")

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/abseil.CMakeLists.txt
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

if(NOT TARGET prometheus-cpp::core)
  message(STATUS "Dependency: prometheus-cpp")

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/prometheus-cpp.CMakeLists.txt
                 ${CMAKE_BINARY_DIR}/prometheus-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                  RESULT_VARIABLE result
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/prometheus-download)
  if(result)
    message(FATAL_ERROR "CMake step failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
                  RESULT_VARIABLE result
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/prometheus-download)
  if(result)
    message(FATAL_ERROR "Build step failed: ${result}")
  endif()

  set(ENABLE_PUSH OFF CACHE BOOL "Build prometheus-cpp push library" FORCE)
  set(ENABLE_PULL OFF CACHE BOOL "Build prometheus-cpp pull library" FORCE)
  set(ENABLE_COMPRESSION OFF
      CACHE BOOL "Enable gzip compression for prometheus-cpp"
      FORCE)
  set(ENABLE_TESTING OFF CACHE BOOL "Build test for prometheus-cpp" FORCE)
  add_subdirectory(${CMAKE_BINARY_DIR}/prometheus-src
                   ${CMAKE_BINARY_DIR}/prometheus-build EXCLUDE_FROM_ALL)
endif()
