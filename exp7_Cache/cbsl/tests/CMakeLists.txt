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
link_libraries(${CBSL_LIB} ${ZSTD_LIB})

add_executable(scalar_data scalar_data.c)
add_executable(single_array single_array.c)
add_executable(multi_array multi_array.c)
add_executable(utility utility.c)
add_executable(split_compression split_compression.c)
add_executable(variable_size_array variable_size_array.c)
add_executable(various_size_array various_size_array.c)

add_executable(cbsl_record       cbsl_record.c)
add_executable(cbsl_record_array cbsl_record_array.c)

add_executable(fortran_bindings fortran_bindings.f90)
target_link_libraries(fortran_bindings ${CBSL_FLIB} ${CBSL_LIB} ${ZSTD_LIB})

add_test(NAME utility COMMAND utility)


#
# very short data compression/decompression
#
add_test(NAME scalar_data_compression   COMMAND scalar_data -c)
add_test(NAME scalar_data_decompression COMMAND scalar_data -d)
set_tests_properties(scalar_data_decompression PROPERTIES DEPENDS scalar_data_compression)


#
# boundary checking with single array
#
function(add_simple_test_size_boundary DATA_SIZE)
  math(EXPR DATA_SIZE_B "${DATA_SIZE} - 1")
  add_test(NAME single_array_${DATA_SIZE_B} COMMAND single_array ${DATA_SIZE_B})
  add_test(NAME single_array_${DATA_SIZE}   COMMAND single_array ${DATA_SIZE})
  math(EXPR DATA_SIZE_B "${DATA_SIZE} + 1")
  add_test(NAME single_array_${DATA_SIZE_B} COMMAND single_array ${DATA_SIZE_B})
endfunction()

foreach(count RANGE 10 20)
  math(EXPR DATA_SIZE "1 << ${count}")
  add_simple_test_size_boundary(${DATA_SIZE})
endforeach()


#
# boundary checking with multiple arrays
#
function(add_multiple_test_size NUM_VARS)
  foreach(count RANGE 10 20)
    math(EXPR DATA_SIZE "1 << ${count}")
    add_test(NAME multi_array_${DATA_SIZE}_by_${NUM_VARS}  COMMAND multi_array ${NUM_VARS} ${DATA_SIZE})
  endforeach()
endfunction()

add_multiple_test_size(2)
add_multiple_test_size(3)
add_multiple_test_size(11)


#
# test compression/decompression form
#
add_test(NAME variable_size_array COMMAND variable_size_array)
math(EXPR DATA_SIZE "1 << 17")
add_test(NAME buffered_read_write COMMAND variable_size_array ${DATA_SIZE})
math(EXPR DATA_SIZE "(1 << 17) + 1")
add_test(NAME immediate_read_write COMMAND variable_size_array ${DATA_SIZE})

add_test(NAME various_size_array COMMAND various_size_array)


add_test(NAME cbsl_record        COMMAND cbsl_record)
add_test(NAME cbsl_record_array  COMMAND cbsl_record_array)


#
# large array compression/decompression by splitting small block
#
math(EXPR BLOCK_SIZE "1 << 18") # 256 KiB
math(EXPR TOTAL_SIZE "1 << 28") # 256 MiB
add_test(NAME split_compression_256KiB_256MiB COMMAND split_compression ${BLOCK_SIZE} ${TOTAL_SIZE})

math(EXPR BLOCK_SIZE "(1 << 18) - (1 << 16)") # 192 KiB
math(EXPR TOTAL_SIZE "1 << 28")               # 256 MiB
add_test(NAME split_compression_192KiB_256MiB COMMAND split_compression ${BLOCK_SIZE} ${TOTAL_SIZE})


#
# Fortran bindings API
#
add_test(NAME fortran_bindings COMMAND fortran_bindings)
