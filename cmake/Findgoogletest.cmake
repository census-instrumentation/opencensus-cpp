# Copyright 2018 The Cartographer Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.2)

if(NOT TARGET gtest)
  set(prefix ${CMAKE_STATIC_LIBRARY_PREFIX})
  set(suffix ${CMAKE_STATIC_LIBRARY_SUFFIX})
  include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
  set(GOOGLETEST_PROJECT_NAME googletest)
  set(GOOGLETEST_PROJECT_SRC_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/${GOOGLETEST_PROJECT_NAME}-prefix/src/${GOOGLETEST_PROJECT_NAME}")
  set(GOOGLETEST_PROJECT_BUILD_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/${GOOGLETEST_PROJECT_NAME}-prefix/src/${GOOGLETEST_PROJECT_NAME}-build")
  set(GOOGLETEST_INCLUDE_DIRS ${GOOGLETEST_PROJECT_SRC_DIR})
  
  include(ExternalProject)
  ExternalProject_Add(${GOOGLETEST_PROJECT_NAME}
    GIT_REPOSITORY https://github.com/abseil/googletest
    GIT_TAG "master"
    INSTALL_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} --build "${GOOGLETEST_PROJECT_BUILD_DIR}"
    CMAKE_CACHE_ARGS "-Dgtest_force_shared_crt:BOOL=ON"
  )
  
  file(MAKE_DIRECTORY ${GOOGLETEST_PROJECT_SRC_DIR}/googletest/include)
  file(MAKE_DIRECTORY ${GOOGLETEST_PROJECT_SRC_DIR}/googlemock/include)

  add_library(gtest STATIC IMPORTED)
  set_target_properties(gtest PROPERTIES
    IMPORTED_LOCATION_DEBUG ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gtest${suffix}
    IMPORTED_LOCATION_RELEASE ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gtestd${suffix}
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES ${GOOGLETEST_PROJECT_SRC_DIR}/googletest/include)
  add_dependencies(gtest googletest)

  add_library(gtest_main STATIC IMPORTED)
  set_target_properties(gtest_main PROPERTIES
    IMPORTED_LOCATION_DEBUG ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gtest_maind${suffix}
    IMPORTED_LOCATION_RELEASE ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gtest_main${suffix}
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES ${GOOGLETEST_PROJECT_SRC_DIR}/googletest/include)
  add_dependencies(gtest_main googletest)

  add_library(gmock STATIC IMPORTED)
  set_target_properties(gmock PROPERTIES
    IMPORTED_LOCATION_DEBUG ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gmockd${suffix}
    IMPORTED_LOCATION_RELEASE ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gmock${suffix}
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES ${GOOGLETEST_PROJECT_SRC_DIR}/googlemock/include)
  add_dependencies(gmock googletest)

  add_library(gmock_main STATIC IMPORTED)
  set_target_properties(gmock_main PROPERTIES
    IMPORTED_LOCATION_DEBUG ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gmock_maind${suffix}
    IMPORTED_LOCATION_RELEASE ${GOOGLETEST_PROJECT_BUILD_DIR}/lib/${prefix}gmock_main${suffix}
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES ${GOOGLETEST_PROJECT_SRC_DIR}/googlemock/include)
  add_dependencies(gmock_main googletest)
  
  unset(prefix)
  unset(suffix)
endif()
