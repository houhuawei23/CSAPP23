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
set(LIB_SOURCES
    buffer.c
    file.c
    utils.c
    flush.c
    write.c
    read.c
    record.c
    )

set(FLIB_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90
    )
configure_file(fortran_bindings.f90.in ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90.in.2)

find_program(CPP cpp)
if (CPP_NOTFOUND)
  message(FATAL_ERROR "C preprocessor (cpp) command not found.")
endif ()
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90
                   COMMAND ${CPP} ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90.in.2 ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90
                   COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/gen_fortran_bindings.cmake ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90.in.2
                  )
add_custom_target(generate_preprocessed_fortran_file SOURCES ${CMAKE_CURRENT_BINARY_DIR}/fortran_bindings.f90)

add_library(${CBSL_LIB}  ${LIB_SOURCES})
add_library(${CBSL_FLIB} ${FLIB_SOURCES})

install(TARGETS ${CBSL_LIB} ${CBSL_FLIB})
install(FILES "${CMAKE_Fortran_MODULE_DIRECTORY}/${CBSL_FLIB}.mod" DESTINATION include)
