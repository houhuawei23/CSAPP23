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
#define CHECK_BINARY(X,Y)    {if (memcmp(&(X),&(Y),sizeof((X))) != 0) { fprintf(stderr, "error: binary check %s == %s\n", (#X), (#Y)); exit(1); }}

typedef unsigned char byte_t;

extern void record(cbsl_mode, int, byte_t*, int);
extern void rand_byte_t(int n, byte_t* a);

char cname[128];

int main(int argc, char** argv)
{
  if (argc < 3)
    return 1;

  sprintf(cname, "split.zst");

  int block_size, total_size;
  sscanf(argv[1], "%d\n", &block_size);
  sscanf(argv[2], "%d\n", &total_size);

  if (total_size < block_size)
    return 2;

  byte_t* a = malloc(sizeof(byte_t) * total_size);
  byte_t* b = malloc(sizeof(byte_t) * total_size);
  rand_byte_t(total_size, a);

  record(cbsl_store_mode, total_size, a, block_size);
  record(cbsl_load_mode,    total_size, b, block_size);

  for (int i = 0; i < total_size; ++i)
  {
    if (a[i] != b[i])
    {
      fprintf(stderr, "mismatch!\n");
      exit(1);
    }
  }

  return 0;
}

void record(cbsl_mode mode, int total_size, byte_t* a, int block_size)
{
  cbsl_ctx* ctx = cbsl_open(mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }

  int total = 0;
  do
  {
    int size = (total_size - total < block_size) ? total_size - total : block_size;
    CBSL_ERROR_CHECK(cbsl_record(ctx, a + total, size));
    total += size;
  }
  while(total < total_size);
  if (total != total_size)
  {
    fprintf(stderr, "total != total_size\n");
    exit(1);
  }

  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void rand_byte_t(int n, byte_t* a) {
  for(int i = 0; i < n; ++i)
    a[i] = rand() % 255;
}
