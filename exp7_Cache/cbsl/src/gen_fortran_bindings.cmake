#
# Copyright 2019 Yuta Hirokawa (University of Tsukuba, Japan)
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
cmake_minimum_required(VERSION 3.3)

if (${CMAKE_ARGC} LESS 4)
  message(FATAL_ERROR "Usage ${CMAKE_ARGV2} <input file> <output file>")
endif ()

set(INPUT_FILE  ${CMAKE_ARGV3})
if (CMAKE_ARGV4)
  set(OUTPUT_FILE ${CMAKE_ARGV4})
else ()
  set(OUTPUT_FILE ${CMAKE_ARGV3})
endif ()

file(READ ${INPUT_FILE} FBIND_SOURCE_TEXT)
string(REGEX REPLACE "__NL__" "\n" FBIND_GEN_TEXT ${FBIND_SOURCE_TEXT})
file(WRITE ${OUTPUT_FILE} ${FBIND_GEN_TEXT})
