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

include(FetchContent)

fetchcontent_declare(googletest
                     GIT_REPOSITORY
                     https://github.com/abseil/googletest
                     GIT_TAG
                     ed2fe122f8dc9aca844d724986d1d5cf5b65ea4e)
fetchcontent_declare(abseil
                     GIT_REPOSITORY
                     https://github.com/abseil/abseil-cpp
                     GIT_TAG
                     master)
fetchcontent_declare(prometheus
                     GIT_REPOSITORY
                     https://github.com/jupp0r/prometheus-cpp
                     GIT_TAG
                     master)

fetchcontent_getproperties(googletest)
if(BUILD_TESTING)
  message(STATUS "Dependency: googletest (BUILD_TESTING=${BUILD_TESTING})")

  if(NOT googletest_POPULATED)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      # All the libraries in the build must use either /MD or /MT (runtime
      # library to link)
      #
      # force this option to ON so that Google Test will use /MD instead of /MT
      # /MD is now the default for Visual Studio, so it should be our default,
      # too
      option(
        gtest_force_shared_crt
        "Use shared (DLL) run-time lib even when Google Test is built as static lib."
        ON)
    endif()

    fetchcontent_populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR}
                     EXCLUDE_FROM_ALL)
  endif()
endif()

fetchcontent_getproperties(abseil)
if(NOT abseil_POPULATED)
  fetchcontent_populate(abseil)
  add_subdirectory(${abseil_SOURCE_DIR} ${abseil_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

fetchcontent_getproperties(prometheus)
if(NOT prometheus_POPULATED)
  set(ENABLE_PUSH OFF CACHE BOOL "Build prometheus-cpp push library" FORCE)
  set(ENABLE_PULL OFF CACHE BOOL "Build prometheus-cpp pull library" FORCE)
  set(ENABLE_COMPRESSION OFF
      CACHE BOOL "Enable gzip compression for prometheus-cpp"
      FORCE)
  set(ENABLE_TESTING OFF CACHE BOOL "Build test for prometheus-cpp" FORCE)
  fetchcontent_populate(prometheus)
  add_subdirectory(${prometheus_SOURCE_DIR} ${prometheus_BINARY_DIR}
                   EXCLUDE_FROM_ALL)
endif()
