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

option(BUILD_STACKDRIVER_EXPORTER "Build the stackdriver exporter" OFF)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(CTest) # Defines option BUILD_TESTING.
enable_testing()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# There are cyclical dependencies ( betwen libopencensus_trace.so and
# libopencensus_context.so ) that prevent from building the code as is as shared
# libraries without undefined symbols...
#
# Moreover, because of the way I built gRPC c++ apis within google-cloud-cpp as
# shared libs, with a specific abseil version _as shared lib_

if(BUILD_SHARED_LIBS)
  set(POSITION_INDEPENDENT_CODE ON)
  set(BUILD_SHARED_LIBS NO)
endif()
# clearly there are problems with cyclical deps in this code ( context, trace )
if(BUILD_SHARED_LIBS)
  add_link_options("LINKER:--no-undefined")
endif()

include(GNUInstallDirs)

include(OpenCensusDeps)

include(OpenCensusHelpers)

if(BUILD_SHARED_LIBS)
  add_link_options("LINKER:--no-undefined")
endif()

# OpenCensus code.
add_subdirectory(opencensus)

# Example code only if testing is enabled.
if(BUILD_TESTING)
  add_subdirectory(examples)
endif()

install(EXPORT opencensus-cpp-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/opencensus-cpp)

# we still need to create and export opencensus-cpp-config.cmake

# Create and install the CMake configuration files.
configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake/config.cmake.in"
               "opencensus-cpp-config.cmake" @ONLY)
configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake/config-version.cmake.in"
               "opencensus-cpp-config-version.cmake" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/opencensus-cpp-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/opencensus-cpp-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/opencensus-cpp")
