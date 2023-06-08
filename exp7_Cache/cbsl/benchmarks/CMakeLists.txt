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
include(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME)

if (HAVE_CLOCK_GETTIME)
  link_libraries(rt)
else ()
  include(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
  if (not HAVE_GETTIMEOFDAY)
    message(WARNING "Benchmark program uses low resolution timer...")
  endif ()
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

link_libraries(${CBSL_LIB} ${ZSTD_LIB} m)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


add_executable(serialize   serialize.c)
add_executable(deserialize deserialize.c)

add_custom_target(benchmark_serialize_bestcase   ./serialize   18 27 -f DEPENDS serialize)
add_custom_target(benchmark_deserialize_bestcase ./deserialize 18 27 -f DEPENDS deserialize benchmark_serialize_bestcase)
add_custom_target(benchmark_fast DEPENDS benchmark_serialize_bestcase benchmark_deserialize_bestcase)

add_custom_target(benchmark_serialize_worstcase   ./serialize   18 27 -r DEPENDS serialize)
add_custom_target(benchmark_deserialize_worstcase ./deserialize 18 27 -r DEPENDS deserialize benchmark_serialize_worstcase)
add_custom_target(benchmark_slow DEPENDS benchmark_serialize_worstcase benchmark_deserialize_worstcase)

add_custom_target(benchmark DEPENDS benchmark_fast benchmark_slow)
