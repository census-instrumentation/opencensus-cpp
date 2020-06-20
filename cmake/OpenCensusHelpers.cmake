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
      list(APPEND _DEPS "$<BUILD_INTERFACE:${dep}>")
    else()
      list(APPEND _DEPS "opencensus_${dep}")
    endif()
  endforeach()
  set(${OUT}
      ${_DEPS}
      PARENT_SCOPE)
endfunction()

# Helper function like bazel's cc_test. Usage:
#
# opencensus_test(trace_some_test internal/some_test.cc dep1 dep2...)
function(opencensus_test NAME SRC)
  if(BUILD_TESTING)
    set(_NAME "opencensus_${NAME}")
    add_executable(${_NAME} ${SRC})
    prepend_opencensus(DEPS "${ARGN}")
    target_link_libraries(${_NAME} "${DEPS}" gmock gtest_main)
    add_test(NAME ${_NAME} COMMAND ${_NAME})
  endif()
endfunction()

# Helper function like bazel's cc_benchmark. Usage:
#
# opencensus_benchmark(trace_some_benchmark internal/some_benchmark.cc dep1
# dep2...)
function(opencensus_benchmark NAME SRC)
  if(BUILD_TESTING)
    set(_NAME "opencensus_${NAME}")
    add_executable(${_NAME} ${SRC})
    prepend_opencensus(DEPS "${ARGN}")
    target_link_libraries(${_NAME} "${DEPS}" benchmark)
  endif()
endfunction()


include(GNUInstallDirs)

#
# install( TARGETS PUBLIC_HEADERS DESTINATION foo )
# does not work well when we declare
# opencensus_lib( libfoo PUBLIC HDRS bar/baz.h  SRCS bar/bar.cc )
# it will generate   foo/baz.h   not foo/bar/baz.hh
#
#
# install_headers_with_subdirectories( PUBLIC_HEADERS item1 item2 .. INSTALL_FOLDER dir EXPORT export_name )
#
function( install_headers_with_subdirectories )
  set(options "" )
  set(singleValued "INSTALL_FOLDER")
  set( multiValued "PUBLIC_HEADER" )

  cmake_parse_arguments(ARG "${options}" "${singleValued}" "${multiValued}" ${ARGN} )

  foreach( header ${ARG_PUBLIC_HEADER} )

    get_filename_component( dir ${header} DIRECTORY )

    install( FILES ${header} DESTINATION "${ARG_INSTALL_FOLDER}/${dir}" )

  endforeach()

endfunction( install_headers_with_subdirectories )

# Helper function like bazel's cc_library.  Libraries are namespaced as
# opencensus_* and public libraries are also aliased as opencensus-cpp::*.
function(opencensus_lib NAME)
  cmake_parse_arguments(ARG "PUBLIC;PRIVATE" "" "HDRS;SRCS;DEPS" ${ARGN})
  set(_NAME "opencensus_${NAME}")
  prepend_opencensus(ARG_DEPS "${ARG_DEPS}")

  string( REPLACE "${PROJECT_SOURCE_DIR}/" ""  _current_dir_relative_path "${CMAKE_CURRENT_LIST_DIR}" )

  if(ARG_SRCS)
    add_library(${_NAME} ${ARG_SRCS})
    target_link_libraries(${_NAME} PUBLIC ${ARG_DEPS})
    target_include_directories(${_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> $<INSTALL_INTERFACE:include> )
  else()
    add_library(${_NAME} INTERFACE)
    target_link_libraries(${_NAME} INTERFACE ${ARG_DEPS})
    target_include_directories(${_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> $<INSTALL_INTERFACE:include> )
  endif()
  if(ARG_PUBLIC)
    add_library(${PROJECT_NAME}::${NAME} ALIAS ${_NAME})

    if (ARG_HDRS)
        #this will install them
        #set_target_properties( ${_NAME} PROPERTIES PUBLIC_HEADER "${ARG_HDRS}" )
        install_headers_with_subdirectories( PUBLIC_HEADER ${ARG_HDRS}
                                             INSTALL_FOLDER "${CMAKE_INSTALL_INCLUDEDIR}/${_current_dir_relative_path}"
					    )
    endif()

    install( TARGETS ${_NAME}
             EXPORT opencensus-cpp-targets
	     RUNTIME DESTINATION       "${CMAKE_INSTALL_BINDIR}"
	     LIBRARY DESTINATION       "${CMAKE_INSTALL_LIBDIR}"
	     ARCHIVE DESTINATION       "${CMAKE_INSTALL_LIBDIR}"
#	     PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_current_dir_relative_path}"
   )
  elseif(ARG_PRIVATE)
    # what we really wanted to do, if linking private static libraries into public ones worked
    # without having to export the private one...
  else()

    if (ARG_HDRS)
       # I think we have API problems whereby the internal includes are required by clients...
       # Comment out this line if it is not the case...
       #set_target_properties( ${_NAME} PROPERTIES PRIVATE_HEADER "${ARG_HDRS}" )
       install_headers_with_subdirectories( PUBLIC_HEADER ${ARG_HDRS}
                                            INSTALL_FOLDER "${CMAKE_INSTALL_INCLUDEDIR}/${_current_dir_relative_path}"
                                          )
    endif()

    # fight export bug ?
    # I don't want these installed, I don't want the associated headers installed
    # I want them as depedencies to public static libraries that aggregate them....
    install( TARGETS ${_NAME}
             EXPORT opencensus-cpp-targets
	     RUNTIME DESTINATION       "${CMAKE_INSTALL_BINDIR}"
	     LIBRARY DESTINATION       "${CMAKE_INSTALL_LIBDIR}"
	     ARCHIVE DESTINATION       "${CMAKE_INSTALL_LIBDIR}"
#	     PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_current_dir_relative_path}"
   )

  endif()
endfunction()

# Helper function for fuzzing. Usage:
#
# opencensus_fuzzer(trace_some_fuzzer internal/some_fuzzer.cc dep1 dep2...)
function(opencensus_fuzzer NAME SRC)
  if(FUZZER)
    set(_NAME "opencensus_${NAME}")
    add_executable(${_NAME} ${SRC})
    prepend_opencensus(DEPS "${ARGN}")
    target_link_libraries(${_NAME} "${DEPS}" ${FUZZER})
    target_compile_options(${_NAME} PRIVATE ${FUZZER})
  endif()
endfunction()
