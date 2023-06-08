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
#include <time.h>
#include <math.h>
#include <cbsl.h>

#define CBSL_ERROR_CHECK(X)  {if ((X) == cbsl_error) { fprintf(stderr, "error: %s\n", (#X)); exit(1); }}

typedef unsigned char byte_t;

extern void compress(uint64_t n, uint64_t* sizes, byte_t** a);
extern void raw_write(uint64_t n, uint64_t* sizes, byte_t** a);
extern void decompress(uint64_t n, uint64_t* sizes, byte_t** a);
extern void raw_read(uint64_t n, uint64_t* sizes, byte_t** a);

extern uint64_t rand_size();
extern void     rand_byte_t(uint64_t data_size, byte_t* a);

char cname[128], sname[128];

int main(int argc, char** argv)
{
  sprintf(cname, "various_compressed.zst");
  sprintf(sname, "various_raw.dat");

  srand((unsigned int)(time(NULL)));

  uint64_t num_vars;
  if (argc < 2)
  {
    num_vars = rand() % 100 + 2;
  }
  else
  {
    sscanf(argv[1], "%lu\n", &num_vars);
  }
  printf("number of array (data sets): %lu\n", num_vars);

  byte_t**  a = (byte_t**)(malloc(sizeof(byte_t**) * num_vars));
  byte_t**  b = (byte_t**)(malloc(sizeof(byte_t**) * num_vars));
  byte_t**  c = (byte_t**)(malloc(sizeof(byte_t**) * num_vars));
  uint64_t* asizes = (uint64_t*)(malloc(sizeof(uint64_t*) * num_vars));
  uint64_t* bsizes = (uint64_t*)(malloc(sizeof(uint64_t*) * num_vars));
  uint64_t* csizes = (uint64_t*)(malloc(sizeof(uint64_t*) * num_vars));

  for(uint64_t i = 0; i < num_vars; ++i)
  {
    asizes[i] = rand_size();
    a[i] = (byte_t*)(malloc(sizeof(byte_t*) * asizes[i]));
    rand_byte_t(asizes[i] / sizeof(byte_t), a[i]);
    printf("array[%lu] = %lu bytes\n", i, asizes[i]);

    b[i] = (byte_t*)(malloc(sizeof(byte_t*) * asizes[i]));
    c[i] = (byte_t*)(malloc(sizeof(byte_t*) * asizes[i]));
  }
  
  compress(num_vars, asizes, a);
  raw_write(num_vars, asizes, a);

  decompress(num_vars, bsizes, b);
  raw_read(num_vars, csizes, c);

  for (uint64_t i = 0; i < num_vars; ++i)
  {
    if (asizes[i] != bsizes[i] || bsizes[i] != csizes[i])
    {
      fprintf(stderr, "array size is mismatch!\n");
      exit(1);
    }
  }

  for (uint64_t i = 0; i < num_vars; ++i)
  for (uint64_t j = 0; j < asizes[i]; ++j)
  {
    if (a[i][j] != b[i][j] || b[i][j] != c[i][j])
    {
      fprintf(stderr, "data value is mismatch! (a,b,c)[%lu][%lu]\n", i, j);
      exit(1);
    }
  }

  for (uint64_t i = 0; i < num_vars; ++i)
  {
    free(a[i]);
    free(b[i]);
    free(c[i]);
  }
  free(a);
  free(b);
  free(c);
  free(asizes);
  free(bsizes);
  free(csizes);

  return 0;
}

void compress(uint64_t n, uint64_t* sizes, byte_t** a)
{
  cbsl_ctx* ctx = cbsl_open(cbsl_store_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open(cbsl_store_mode)\n");
    exit(1);
  }
  for (uint64_t i = 0; i < n; ++i)
  {
    CBSL_ERROR_CHECK(cbsl_write(ctx, &sizes[i], sizeof(sizes[i])));
    CBSL_ERROR_CHECK(cbsl_write(ctx, a[i], sizes[i]));
  }
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void raw_write(uint64_t n, uint64_t* sizes, byte_t** a)
{
  FILE* fp = fopen(sname, "wb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen(wb)\n");
    exit(1);
  }
  for (uint64_t i = 0; i < n; ++i)
  {
    fwrite(&sizes[i], 1, sizeof(sizes[i]), fp);
    fwrite(a[i], 1, sizes[i], fp);
  }
  fclose(fp);
}

void decompress(uint64_t n, uint64_t* sizes, byte_t** a)
{
  cbsl_ctx* ctx = cbsl_open(cbsl_load_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open(cbsl_load_mode)\n");
    exit(1);
  }
  for (uint64_t i = 0; i < n; ++i)
  {
    CBSL_ERROR_CHECK(cbsl_read(ctx, &sizes[i], sizeof(sizes[i])));
    CBSL_ERROR_CHECK(cbsl_read(ctx, a[i], sizes[i]));
  }
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void raw_read(uint64_t n, uint64_t* sizes, byte_t** a)
{
  FILE* fp = fopen(sname, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen(rb)\n");
    exit(1);
  }
  for (uint64_t i = 0; i < n; ++i)
  {
    fread(&sizes[i], 1, sizeof(sizes[i]), fp);
    fread(a[i], 1, sizes[i], fp);
  }
  fclose(fp);
}

uint64_t rand_size()
{
  const uint64_t min_data_size = 1024;
  const uint64_t max_data_size = 1048576; /* 1 MiB */
  uint64_t size = rand() % max_data_size;
  return (size < min_data_size) ? min_data_size : size;
}

void rand_byte_t(uint64_t data_size, byte_t* a)
{
  for(uint64_t i = 0; i < data_size; ++i)
    a[i] = rand() % 255;
}
