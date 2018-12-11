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

# Prepends opencensus_ to all deps that aren't in a :: namespace.
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

# Helper function like bazel's cc_test. Usage:
#
# opencensus_test(trace_some_test internal/some_test.cc dep1 dep2...)
function(opencensus_test NAME SRC)
  if(BUILD_TESTING)
    set(_NAME "opencensus_${NAME}")
    add_executable(${_NAME} ${SRC})
    prepend_opencensus(DEPS "${ARGN}")
    target_link_libraries(${_NAME} PRIVATE "${DEPS}" gmock gtest_main)
    add_test(NAME ${_NAME} COMMAND ${_NAME})
  endif()
endfunction()

# Helper function like bazel's cc_library.  Libraries are namespaced as
# opencensus_* and public libraries are also aliased as opencensus-cpp::*.
function(opencensus_lib NAME)
  cmake_parse_arguments(ARG "PUBLIC" "" "HDRS;SRCS;DEPS;PRIVATE_DEPS" ${ARGN})
  set(_NAME "opencensus_${NAME}")
  prepend_opencensus(ARG_DEPS "${ARG_DEPS}")
  
  if(ARG_SRCS)
    add_library(${_NAME} ${ARG_SRCS})
    if(ARG_DEPS)
      target_link_libraries(${_NAME} PUBLIC ${ARG_DEPS})
    endif()
    if(ARG_PRIVATE_DEPS)
      target_link_libraries(${_NAME} PRIVATE ${ARG_PRIVATE_DEPS})
    endif()
    target_include_directories(${_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
  else()
    add_library(${_NAME} INTERFACE)
    target_link_libraries(${_NAME} INTERFACE ${ARG_DEPS})
    target_include_directories(${_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
  endif()
  
  if(INSTALL_HEADERS)
    if(ARG_HEADERS)
      message(STATUS "Arg_headers: ${ARG_HEADERS}")
      foreach(header_file IN LISTS ARG_HEADERS)
          get_filename_component(dir ${header_file} DIRECTORY)
          install(FILES ${CMAKE_SOURCE_DIR}/${header_file} DESTINATION "include/${dir}")
      endforeach()
    endif()
  endif()
  
  install(TARGETS ${_NAME} EXPORT opencensus-targets
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
  )
endfunction()
