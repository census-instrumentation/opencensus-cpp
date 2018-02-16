#! /usr/bin/env bash
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

# Build/run only targets affected by files changed in the commit range,
# according to bazel query. Based on
# https://github.com/bazelbuild/bazel/blob/master/scripts/ci/ci.sh

files=()
# If WORKSPACE or .travis.yml is touched, almost anything may be affected.
# Otherwise, top-level files are irrelevant to the build and we exclude them to
# suppress query errors.
if [[ ! -z $(git diff --name-only ${TRAVIS_COMMIT_RANGE} \
  | grep "WORKSPACE\|.travis.yml" ) ]];
then
  echo ".travis.yml or WORKSPACE affected; running all tests."
  files=("//...")
else
  echo "Affected files:"
  for file in $(git diff --name-only ${TRAVIS_COMMIT_RANGE} | grep / ); do
    mapfile -O ${#files[@]} -t files <<< "$(bazel query $file)"
    bazel query $file
  done
fi

# We can't use --noshow_progress on build/test commands because Travis
# terminates the build after 10 mins without output.
buildables=$(bazel query -k --noshow_progress \
  "kind(rule, rdeps(//..., set(${files[*]})))" \
  | grep -v :_)
if [[ ! -z $buildables ]]; then
  echo "Building targets"
  echo "$buildables"
  bazel build --experimental_ui_actions_shown=1 -k $buildables
fi

# Exclude tests tagged "noci". Tests marked "manual" are already excluded from
# wildcard queries.
tests=$(bazel query -k --noshow_progress \
  "kind(test, rdeps(//..., set(${files[*]}))) except attr('tags', 'noci', //...)" \
  | grep -v :_)
if [[ ! -z $tests ]]; then
  echo "Running tests"
  echo "$tests"
  bazel test --experimental_ui_actions_shown=1 -k $tests
fi
