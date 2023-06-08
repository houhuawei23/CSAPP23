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

enable_testing()

set(CMAKE_C_STANDARD 99)

project(cbsl
        VERSION   2019.5.0
        LANGUAGES C Fortran
        )


if (CMAKE_BUILD_TYPE)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
  if (NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  endif()

  if (${uppercase_CMAKE_BUILD_TYPE} MATCHES "DEBUG")
    set(CBSL_DEBUG 1)
  endif ()
endif ()


if (INSTALL_ZSTD)
  message(STATUS "Enable installation Zstandard library version 1.4.0")
  include(ExternalProject)
  ExternalProject_Add(zstd
    GIT_REPOSITORY   "https://github.com/facebook/zstd.git"
    GIT_TAG          "v1.4.0"
    PREFIX           "${CMAKE_BINARY_DIR}/zstd"
    SOURCE_SUBDIR    "build/cmake"
    CMAKE_ARGS       -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    STEP_TARGETS     install
    EXCLUDE_FROM_ALL on
  )
  include_directories(${CMAKE_INSTALL_PREFIX}/include)
  link_directories(${CMAKE_INSTALL_PREFIX}/lib)
endif ()
find_library(ZSTD_LIB NAMES "zstd")


include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG("-Wall"            C_HAS_WALL)
CHECK_C_COMPILER_FLAG("-Wshadow"         C_HAS_WSHADOW)
CHECK_C_COMPILER_FLAG("-Werror"          C_HAS_WERROR)
CHECK_C_COMPILER_FLAG("-pedantic-errors" C_HAS_PEDANTIC_ERRORS)

set(ADD_C_EXTRA_FLAGS)

if (C_HAS_WALL)
  set(ADD_C_EXTRA_FLAGS "${ADD_C_EXTRA_FLAGS} -Wall")
endif ()

if (C_HAS_WSHADOW)
  set(ADD_C_EXTRA_FLAGS "${ADD_C_EXTRA_FLAGS} -Wshadow")
endif ()

if (C_HAS_WERROR)
  set(ADD_C_EXTRA_FLAGS "${ADD_C_EXTRA_FLAGS} -Werror")
endif ()

if (C_HAS_PEDANTIC_ERRORS)
  set(ADD_C_EXTRA_FLAGS "${ADD_C_EXTRA_FLAGS} -pedantic-errors")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_C_EXTRA_FLAGS}")


include(CheckFortranCompilerFlag)
CHECK_Fortran_COMPILER_FLAG("-Wall -Wextra"    Fortran_HAS_WALL_EXTRA)
CHECK_Fortran_COMPILER_FLAG("-Wshadow"         Fortran_HAS_WERROR)
CHECK_Fortran_COMPILER_FLAG("-pedantic-errors" Fortran_HAS_PEDANTIC_ERRORS)

set(ADD_Fortran_EXTRA_FLAGS)

if (Fortran_HAS_WALL_EXTRA)
  set(ADD_Fortran_EXTRA_FLAGS "${ADD_Fortran_EXTRA_FLAGS} -Wall -Wextra")
endif ()

if (Fortran_HAS_WERROR)
  set(ADD_Fortran_EXTRA_FLAGS "${ADD_Fortran_EXTRA_FLAGS} -Werror")
endif ()

if (Fortran_HAS_PEDANTIC_ERRORS)
  set(ADD_Fortran_EXTRA_FLAGS "${ADD_Fortran_EXTRA_FLAGS} -pedantic-errors")
endif ()

set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${ADD_Fortran_EXTRA_FLAGS}")


include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)
set(CMAKE_Fortran_MODULE_DIRECTORY ${PROJECT_BINARY_DIR}/include)


set(CBSL_LIB  "cbsl")
set(CBSL_FLIB "${CBSL_LIB}f")

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(benchmarks)

if (INSTALL_ZSTD)
  # install zstd before building the library
  add_dependencies(${CBSL_FLIB} ${CBSL_LIB})
  add_dependencies(${CBSL_LIB}  zstd-install)
endif ()
