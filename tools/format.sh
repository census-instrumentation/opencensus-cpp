#!/usr/bin/env bash
# Copyright 2017, OpenCensus Authors
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
#
# Formats code, exits with 0 if there were no changes.
if [[ ! -e tools/format.sh ]]; then
  echo "This tool must be run from the topmost OpenCensus directory." >&2
  exit 1
fi
set -e
# Correct common miscapitalizations.
FIND="find . -name .git -prune -o -name build -prune -o"
sed -i 's/Open[c]ensus/OpenCensus/g' $($FIND -type f -print)
sed -i 's/Stack[D]river/Stackdriver/g' $($FIND -type f -print)
# No trailing spaces.
sed -i 's/ \+$//' $($FIND -type f -print)
# For easier debugging: print the version because it affects the formatting.
CMD=clang-format
$CMD -version
$CMD -i -style=Google \
  $($FIND -name '*.cc' -o -name '*.h' -print)
if which buildifier >/dev/null; then
  echo "Running buildifier."
  buildifier $($FIND -name WORKSPACE -o -name BUILD -o -name '*.bzl' -print)
else
  echo "Can't find buildifier. It can be installed with:"
  echo "  go get github.com/bazelbuild/buildtools/buildifier"
fi
if which cmake-format >/dev/null; then
  echo "Running cmake-format $(cmake-format --version 2>&1)."
  cmake-format -i $($FIND -name '*CMakeLists.txt' -o -name '*.cmake' -print)
else
  echo "Can't find cmake-format. It can be installed with:"
  echo "  pip install --user cmake_format"
fi
CHANGED="$(git ls-files --modified)"
if [[ ! -z "$CHANGED" ]]; then
  echo "The following files have changes:"
  echo "$CHANGED"
  exit 1
else
  echo "No changes."
fi
