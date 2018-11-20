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

# Use ccache if it's present.
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

project(opencensus-cpp VERSION 0.3.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
find_package(Threads REQUIRED)
set(OPENCENSUS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include(CTest) # Defines option BUILD_TESTING.
enable_testing()

# Dependencies.
add_subdirectory(cmake)

# OpenCensus code.
add_subdirectory(opencensus)

# Example code.
add_subdirectory(examples)
