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

# Specify structure for custom cmake functions
additional_commands = {
  "opencensus_lib": {
    "flags": [
      "PUBLIC",
    ],
    "kwargs": {
      "HEADERS": "*",
      "DEPENDS": "*",
      "SOURCES": "*",
    },
  },

  "externalproject_add": {  # Must be lowercase.
    "flags": [],
    "kwargs": {
      "GIT_REPOSITORY": "+",
      "GIT_TAG": "+",
      "SOURCE_DIR": "+",
      "BINARY_DIR": "+",
      "UPDATE_COMMAND": "+",
      "PATCH_COMMAND": "+",
      "CONFIGURE_COMMAND": "+",
      "BUILD_COMMAND": "+",
      "INSTALL_COMMAND": "+",
      "TEST_COMMAND": "+",
      "LOG_DOWNLOAD": 1,
    },
  },
}

# If comment markup is enabled, don't reflow the first comment block in
# eachlistfile. Use this to preserve formatting of your
# copyright/licensestatements.
first_comment_is_literal = True
