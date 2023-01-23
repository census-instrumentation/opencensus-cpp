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

cmake_minimum_required(VERSION 3.5)

# Use ccache if it's present.
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

project(
  opencensus-cpp
  VERSION 0.5.0
  LANGUAGES CXX)

option(FUZZER "Either OFF or e.g. -fsanitize=fuzzer,address" OFF)
option(OpenCensus_BUILD_TESTING "build test and example" OFF)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(CTest) # Defines option BUILD_TESTING.
enable_testing()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(OpenCensusDeps)

include(OpenCensusHelpers)

# OpenCensus code.
add_subdirectory(opencensus)

# Example code only if testing is enabled.
if(BUILD_TESTING AND OpenCensus_BUILD_TESTING)
  add_subdirectory(examples)
endif()

install(DIRECTORY ${CMAKE_SOURCE_DIR}/opencensus TYPE INCLUDE FILES_MATCHING PATTERN "*.h")

install(EXPORT OpenCensusTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenCensus
  NAMESPACE ${PROJECT_NAME}::)

include(CMakePackageConfigHelpers)

configure_package_config_file(cmake/OpenCensusConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/OpenCensusConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenCensus)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OpenCensusConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenCensus)
