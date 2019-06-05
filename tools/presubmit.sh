#!/usr/bin/env bash
#
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

# presubmit:
# Run a slow but complete check of the code to make sure:
#   - Everything builds.
#   - Tests pass.
#   - Sanitizers pass.
#
# This is intended to be run manually and locally, not as part of
# continuous integration.

readonly R="======================================="
readonly BOLD="\\033[1m"
readonly ERR="\\033[31;1m"
readonly NORMAL="\\033[0m"

function run() {
  echo ""
  echo -e "${BOLD}${R}${R}"
  echo "Running: $@"
  echo -e "${R}${R}${NORMAL}"
  $@
  ret="$?"
  if [[ "${ret}" -ne 0 ]]; then
    echo ""
    echo -e "${ERR}>>> Error: returned code ${ret} <<<${NORMAL}"
    exit ${ret}
  fi
}

t0="$(date +%s)"

run bazel build //...
run bazel test //...

run bazel build -c opt //...
run bazel test -c opt //...

for config in asan ubsan; do
  run bazel test --config=$config //...
  run bazel test --config=$config -c opt //...
done

t1="$(date +%s)"
echo ""
echo "Succeeded after $((t1 - t0)) secs."
