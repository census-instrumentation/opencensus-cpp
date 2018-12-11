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

if(NOT TARGET absl_base)
  set(prefix ${CMAKE_STATIC_LIBRARY_PREFIX})
  set(suffix ${CMAKE_STATIC_LIBRARY_SUFFIX})
  include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
  set(ABSEIL_PROJECT_NAME abseil)
  set(ABSEIL_PROJECT_SRC_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/${ABSEIL_PROJECT_NAME}/src/${ABSEIL_PROJECT_NAME})
  set(ABSEIL_PROJECT_BUILD_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/${ABSEIL_PROJECT_NAME}/src/${ABSEIL_PROJECT_NAME}-build)
  set(ABSEIL_INCLUDE_DIRS ${ABSEIL_PROJECT_SRC_DIR})
  
  ExternalProject_Add(${ABSEIL_PROJECT_NAME}
    PREFIX ${ABSEIL_PROJECT_NAME}
    GIT_REPOSITORY   https://github.com/abseil/abseil-cpp.git
    GIT_TAG          5441bbe1db5d0f2ca24b5b60166367b0966790af
    INSTALL_COMMAND  ""
    BUILD_COMMAND    ${CMAKE_COMMAND} --build "${ABSEIL_PROJECT_BUILD_DIR}"
    CMAKE_CACHE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON;-DBUILD_TESTING:BOOL=OFF"
    BUILD_BYPRODUCTS "${ABSEIL_LIBRARY_PATH};${ABSEIL_DEPENDENT_LIBRARIES}"
  )
  
  # absl_algorithm
  
  add_library(absl_algorithm STATIC IMPORTED GLOBAL)
  set_target_properties(absl_algorithm
    PROPERTIES 
	INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
	IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/algorithm/${prefix}absl_algorithm${suffix}
  )
  
  # absl_base
  
  add_library(absl_base STATIC IMPORTED GLOBAL)
  set_target_properties(absl_base
    PROPERTIES IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/base/${prefix}absl_base${suffix}
    INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_PROJECT_SRC_DIR}
  )
    
  set_property(TARGET absl_base
    PROPERTY INTERFACE_LINK_LIBRARIES
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/base/${prefix}absl_dynamic_annotations${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/base/${prefix}absl_malloc_internal${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/base/${prefix}absl_spinlock_wait${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/base/${prefix}absl_throw_delegate${suffix}
  )
  
  # absl_container
  
  add_library(absl_container STATIC IMPORTED GLOBAL)
  set_target_properties(absl_container
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/container/${prefix}absl_container${suffix}
  )
  set_property(TARGET absl_container
    PROPERTY INTERFACE_LINK_LIBRARIES
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/container/${prefix}test_instance_tracker_lib${suffix}
  )
  
  # absl_debugging
  
  add_library(absl_debugging STATIC IMPORTED GLOBAL)
  set_target_properties(absl_debugging
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/debugging/${prefix}absl_debugging${suffix}
  )
  set_property(TARGET absl_debugging
    PROPERTY INTERFACE_LINK_LIBRARIES
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/debugging/${prefix}absl_symbolize${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/debugging/${prefix}absl_examine_stack${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/debugging/${prefix}absl_failure_signal_handler${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/debugging/${prefix}absl_leak_check${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/debugging/${prefix}absl_stack_consumption${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/debugging/${prefix}absl_stacktrace${suffix}
  ) 
  
  # absl_memory
  
  add_library(absl_memory STATIC IMPORTED GLOBAL)
  set_target_properties(absl_memory
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/memory/${prefix}absl_memory${suffix}
  )
  
  # absl_meta
  
  add_library(absl_meta STATIC IMPORTED GLOBAL)
  set_target_properties(absl_meta
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/meta/${prefix}absl_meta${suffix}
  )
  
  # absl_numeric
  
  add_library(absl_numeric STATIC IMPORTED GLOBAL)
  set_target_properties(absl_numeric
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/numeric/${prefix}absl_numeric${suffix}
  )
  set_property(TARGET absl_numeric
    PROPERTY INTERFACE_LINK_LIBRARIES
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/numeric/${prefix}absl_int128${suffix}
  )
  
  # absl_strings
  
  add_library(absl_strings STATIC IMPORTED GLOBAL)
  set_target_properties(absl_strings
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/strings/${prefix}absl_strings${suffix}
  )
  set_property(TARGET absl_strings
    PROPERTY INTERFACE_LINK_LIBRARIES
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/strings/${prefix}str_format_internal${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/strings/${prefix}str_format_extension_internal${suffix}
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/strings/${prefix}absl_str_format${suffix}
  )
  
  # absl_synchronization
  
  add_library(absl_synchronization STATIC IMPORTED GLOBAL)
  set_target_properties(absl_synchronization
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/synchronization/${prefix}absl_synchronization${suffix}
  )
  
  # absl_time
  
  add_library(absl_time STATIC IMPORTED GLOBAL)
  set_target_properties(absl_time
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/time/${prefix}absl_time${suffix}
  )
  
  # absl_utility
  
  add_library(absl_utility STATIC IMPORTED GLOBAL)
  set_target_properties(absl_utility
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/utility/${prefix}absl_utility${suffix}
  )
  
  # absl_span
  
  add_library(absl_span STATIC IMPORTED GLOBAL)
  set_target_properties(absl_span
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/types/${prefix}absl_span${suffix}
  )
  
  # absl_optional
  
  add_library(absl_optional STATIC IMPORTED GLOBAL)
  set_target_properties(absl_optional
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/types/${prefix}absl_optional${suffix}
  )
  
  # absl_variant
  
  add_library(absl_variant STATIC IMPORTED GLOBAL)
  set_target_properties(absl_variant
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    ${ABSEIL_INCLUDE_DIRS}
    IMPORTED_LOCATION
    ${ABSEIL_PROJECT_BUILD_DIR}/absl/types/${prefix}absl_variant${suffix}
  )
  
  
  # missing: absl_hash "${ABSEIL_PROJECT_BUILD_DIR}/absl/hash/${prefix}absl_hash${suffix}"
  
  # absl_types
  
  #add_library(absl_types STATIC IMPORTED GLOBAL)
  #set_target_properties(absl_types
  #  PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
  #  ${ABSEIL_INCLUDE_DIRS}
  #)
  #set_target_properties(absl_types
   # PROPERTIES INTERFACE_LINK_LIBRRARIES
   # "${ABSEIL_PROJECT_BUILD_DIR}/absl/types/${prefix}absl_any${suffix}
   # ${ABSEIL_PROJECT_BUILD_DIR}/absl/types/${prefix}absl_bad_any_cast${suffix}
   # ${ABSEIL_PROJECT_BUILD_DIR}/absl/types/${prefix}absl_bad_optional_access${suffix}
   # 
  #  
  #  "
  #)
  #add_dependencies(absl_types ${ABSEIL_PROJECT_NAME}) 
  #add_library(absl::types ALIAS absl_types)
  
  target_link_libraries(absl_algorithm INTERFACE absl_base absl_meta) 
  target_link_libraries(absl_container INTERFACE absl_algorithm absl_base absl_memory)
  target_link_libraries(absl_debugging INTERFACE absl_base)
  target_link_libraries(absl_memory INTERFACE absl_meta) 
  target_link_libraries(absl_meta INTERFACE absl_base) 
  target_link_libraries(absl_numeric INTERFACE absl_base) 
  target_link_libraries(absl_strings INTERFACE absl_base absl_memory absl_meta absl_numeric) 
  target_link_libraries(absl_synchronization INTERFACE absl_base absl_time absl_debugging) 
  target_link_libraries(absl_time INTERFACE absl_base absl_numeric)
  target_link_libraries(absl_utility INTERFACE absl_base absl_meta) 
  target_link_libraries(absl_span INTERFACE absl_base absl_utility absl_meta absl_algorithm absl_strings) 
  target_link_libraries(absl_optional INTERFACE absl_utility absl_meta absl_algorithm absl_strings) 
  target_link_libraries(absl_variant INTERFACE absl_utility absl_meta absl_algorithm absl_strings) 

  add_dependencies(absl_algorithm ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_base ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_container ${ABSEIL_PROJECT_NAME})
  add_dependencies(absl_debugging ${ABSEIL_PROJECT_NAME})
  add_dependencies(absl_memory ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_meta ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_numeric ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_strings ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_synchronization ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_time ${ABSEIL_PROJECT_NAME})
  add_dependencies(absl_utility ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_span ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_optional ${ABSEIL_PROJECT_NAME}) 
  add_dependencies(absl_variant ${ABSEIL_PROJECT_NAME}) 

  add_library(absl::algorithm ALIAS absl_algorithm)
  add_library(absl::base ALIAS absl_base) 
  add_library(absl::container ALIAS absl_container)
  add_library(absl::debugging ALIAS absl_debugging)
  add_library(absl::memory ALIAS absl_memory)
  add_library(absl::meta ALIAS absl_meta)
  add_library(absl::numeric ALIAS absl_numeric)
  add_library(absl::strings ALIAS absl_strings)
  add_library(absl::synchronization ALIAS absl_synchronization) 
  add_library(absl::time ALIAS absl_time)
  add_library(absl::utility ALIAS absl_utility)
  add_library(absl::span ALIAS absl_span)
  add_library(absl::optional ALIAS absl_optional)
  add_library(absl::variant ALIAS absl_variant)

  unset(prefix)
  unset(suffix)
endif()
