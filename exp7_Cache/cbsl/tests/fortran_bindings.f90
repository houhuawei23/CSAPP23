!
! Copyright 2019 Yuta Hirokawa (University of Tsukuba, Japan)
!
! Licensed under the Apache License, Version 2.0 (the "License");
! you may not use this file except in compliance with the License.
! You may obtain a copy of the License at
!
!     http://www.apache.org/licenses/LICENSE-2.0
!
! Unless required by applicable law or agreed to in writing, software
! distributed under the License is distributed on an "AS IS" BASIS,
! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
! See the License for the specific language governing permissions and
! limitations under the License.
!
program test_fortran_bindings
  use cbslf
  integer, parameter :: array_size = 100000
  real :: source_stack_array(array_size), dest_stack_array(array_size)
  real, allocatable :: source_heap_array(:), dest_heap_array(:), dest_heap_array2(:)
  character(128) :: source_string, dest_string
  character(*), parameter :: cname = 'data_fortran.zst'

  allocate(source_heap_array(array_size))
  allocate(dest_heap_array(array_size))
  call random_number(source_stack_array)
  call random_number(source_heap_array)

  source_string = 'abcdefgABCDEFG%'
  dest_string   = ''

  print *, 'serialize...'
  call serialize

  print *, 'deserialize...'
  call deserialize

  print *, 'compare...'
  if (.not. compare_real_array(array_size, source_stack_array, dest_stack_array)) then
    print *, 'fail: compare stack array'
    stop 1
  end if

  if (.not. compare_real_array(array_size, source_heap_array, dest_heap_array)) then
    print *, 'fail: compare heap array'
    stop 1
  end if

  if (.not. compare_real_array(array_size, source_heap_array, dest_heap_array2)) then
    print *, 'fail: compare heap array'
    stop 1
  end if

  if (trim(source_string) /= trim(dest_string)) then
    print *, 'fail: compare string'
    stop 1
  end if

contains
  subroutine serialize
    implicit none
    type(cbslf_context) :: ctx
    integer(4) :: errcode

    ctx = cbslf_open(cbslf_store_mode, cname, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_open(store)'
      stop 1
    end if

    call cbslf_set_compression_level(ctx, 10, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_set_compression_level'
      stop 1
    end if

    call cbslf_write(ctx, source_stack_array, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_write(stack array)'
      stop 1
    end if

    call cbslf_write(ctx, source_heap_array, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_write(heap array)'
      stop 1
    end if

    call cbslf_write(ctx, source_heap_array, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_write(heap array2)'
      stop 1
    end if

    call cbslf_write(ctx, source_string, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_write(string)'
      stop 1
    end if

    call cbslf_close(ctx)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_close'
      stop 1
    end if
  end subroutine

  subroutine deserialize
    implicit none
    type(cbslf_context) :: ctx
    integer(4) :: errcode
    ctx = cbslf_open(cbslf_load_mode, cname, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_open(load)'
      stop 1
    end if

    call cbslf_read(ctx, dest_stack_array, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_read(stack array)'
      stop 1
    end if

    call cbslf_read(ctx, dest_heap_array, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_read(heap array)'
      stop 1
    end if

    call cbslf_record_heap(ctx, dest_heap_array2, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_record_heap(heap array2)'
      stop 1
    end if

    call cbslf_read(ctx, dest_string, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_read(string)'
      stop 1
    end if

    call cbslf_close(ctx, errcode)
    if (errcode /= cbslf_success) then
      print *, 'fail: cbslf_close'
      stop 1
    end if
  end subroutine

  function compare_real_array(n, a, b) result(ret)
    implicit none
    integer, intent(in) :: n
    real,    intent(in) :: a(n), b(n)
    logical :: ret
    integer :: i
    do i=1,n
      ret = (abs(a(i) - b(i)) <= epsilon(a(i)))
    end do
  end function
end program
