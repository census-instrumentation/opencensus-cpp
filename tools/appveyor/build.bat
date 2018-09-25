REM Copyright 2018, OpenCensus Authors
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM     http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.

REM We need this to append a variable within a loop.
setlocal enabledelayedexpansion

REM TODO: Is there an easier way to convert lines to a space-separated list?
SET BUILDABLES=
FOR /F usebackq %%T IN (`bazel query "kind(rule, //...)" ^| FINDSTR /C:"\:_" /V`) DO (
    SET BUILDABLES=!BUILDABLES! %%T
)

REM TODO: enable the full build when errors are resolved.
bazel build //opencensus/trace //opencensus/stats
REM bazel build %BUILDABLES%

IF %ERRORLEVEL% NEQ 0 EXIT /b %ERRORLEVEL%
EXIT /b 0
