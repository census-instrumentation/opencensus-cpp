#!/usr/bin/env bash
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
set -e
set -x

# Install buildifier if it's not present. It needs at least go 1.8.
if ! which buildifier >/dev/null; then
  eval "$(gimme 1.11)"
  go get -v github.com/bazelbuild/buildtools/buildifier
fi
# Install cmake-format.
pip install --user 'cmake_format>=0.5.2'
# Check format.
tools/format.sh
