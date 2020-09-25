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

# Install buildifier.
wget -O $HOME/bin/buildifier https://github.com/bazelbuild/buildtools/releases/download/2.2.1/buildifier
chmod +x $HOME/bin/buildifier
# Install cmake-format.
pyenv global 3.7.1
pip3 install --user 'cmake_format>=0.5.2'
# Check format.
tools/format.sh
