/*
 * Copyright 2019 Yuta Hirokawa (University of Tsukuba, Japan)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbsl.h>

#define CBSL_ERROR_CHECK(X)  {if ((X) == cbsl_error) { fprintf(stderr, "error: %s\n", (#X)); exit(1); }}
#define CHECK(X)             {if (!(X)) { fprintf(stderr, "error: check %s\n", (#X)); exit(1); }}
#define CHECK_BINARY(X,Y)    {if (memcmp(&(X),&(Y),sizeof((X))) != 0) { fprintf(stderr, "error: binary check %s == %s\n", (#X), (#Y)); exit(1); }}

typedef unsigned char byte_t;

extern void rand_byte_t(uint64_t data_size, byte_t* a);
extern void record(cbsl_mode, uint64_t* size, void** data);

char cname[128];

int main(int argc, char** argv)
{
  sprintf(cname, "check_record_scalar.zst");

  uint64_t size = 1024 * 1024 * sizeof(byte_t);
  byte_t*  data = (byte_t*)(malloc(size));

  rand_byte_t(size, data);
  record(cbsl_store_mode, &size, (void**) &data);

  uint64_t rsize = 0;
  byte_t*  rdata = NULL;
  record(cbsl_load_mode, &rsize, (void**) &rdata);

  CHECK(size == rsize);
  CHECK(rdata != NULL);
  for (uint64_t i = 0; i < (size/sizeof(byte_t)); ++i)
  {
    if (data[i] != rdata[i])
    {
      fprintf(stderr, "1: mismatch!\n");
      exit(1);
    }
    else
    {
      rdata[i] = 0;
    }
  }

  CHECK(size == rsize);
  CHECK(rdata != NULL);
  record(cbsl_load_mode, &rsize, (void**) &rdata);

  for (uint64_t i = 0; i < (size/sizeof(byte_t)); ++i)
  {
    if (data[i] != rdata[i])
    {
      fprintf(stderr, "2: mismatch!\n");
      exit(1);
    }
  }

  free(data);
  free(rdata);

  return 0;
}

void record(cbsl_mode mode, uint64_t* size, void** data)
{
  cbsl_ctx* ctx = cbsl_open(mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_record_heap(ctx, data, size));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void rand_byte_t(uint64_t data_size, byte_t* a)
{
  for(uint64_t i = 0; i < data_size; ++i)
    a[i] = rand() % 255;
}
