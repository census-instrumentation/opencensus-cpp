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
# Formats code under opencensus/, exits with 0 if there were no changes.
if [[ ! -e tools/format.sh ]]; then
  echo "This tool must be run from the topmost OpenCensus directory." >&2
  exit 1
fi
set -e
# Correct a common miscapitalization.
sed -i 's/Open[c]ensus/OpenCensus/g' $(find * -type f)
# For easier debugging: print the version because it affects the formatting.
CMD=clang-format
$CMD -version
$CMD -i -style=Google $(find . -name '*.cc' -or -name '*.h')
CHANGED="$(git ls-files --modified)"
if [[ ! -z "$CHANGED" ]]; then
  echo "The following files have changes:"
  echo "$CHANGED"
  exit 1
else
  echo "No changes."
fi
