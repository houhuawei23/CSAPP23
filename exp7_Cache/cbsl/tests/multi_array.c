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

extern void compress(uint64_t num_vars, uint64_t data_size, byte_t** a);
extern void raw_write(uint64_t num_vars, uint64_t data_size, byte_t** a);
extern void decompress(uint64_t num_vars, uint64_t data_size, byte_t** a);
extern void raw_read(uint64_t num_vars, uint64_t data_size, byte_t** a);
extern void rand_byte_t(uint64_t data_size, byte_t* a);

char cname[128], sname[128];

int main(int argc, char** argv)
{
  if (argc <= 2)
    exit(1);

  uint64_t data_size, num_vars;
  sscanf(argv[1], "%lu", &num_vars);
  sscanf(argv[2], "%lu", &data_size);
  data_size = data_size / num_vars;

  printf("variables = %lu\n", num_vars);
  printf("data size = %lf [MiB] * %lu\n", (double)(data_size)/pow(2,20), num_vars);

  sprintf(cname, "multiple_compressed_%lu.zst", data_size * num_vars);
  sprintf(sname, "multiple_raw_%lu.dat", data_size);

  srand((unsigned int)(time(NULL)));

  byte_t** a = malloc(sizeof(byte_t*) * num_vars);
  byte_t** b = malloc(sizeof(byte_t*) * num_vars);
  byte_t** c = malloc(sizeof(byte_t*) * num_vars);
  for(uint64_t i = 0; i < num_vars; ++i)
  {
    a[i] = (byte_t*)(malloc(data_size));
    rand_byte_t(data_size / sizeof(byte_t), a[i]);
    b[i] = (byte_t*)(malloc(data_size));
    c[i] = (byte_t*)(malloc(data_size));
  }

  compress(num_vars, data_size, a);
  raw_write(num_vars, data_size, a);

  decompress(num_vars, data_size, b);
  raw_read(num_vars, data_size, c);

  for(uint64_t i = 0; i < num_vars; ++i)
  {
    for(uint64_t j = 0; j < data_size; ++j)
    {
      if (b[i][j] != c[i][j])
      {
        fprintf(stderr, "mismatch!\n");
        exit(1);
      }
    }
  }

  for(uint64_t i = 0; i < num_vars; ++i)
  {
    free(a[i]);
    free(b[i]);
    free(c[i]);
  }
  free(a);
  free(b);
  free(c);

  return 0;
}

void compress(uint64_t num_vars, uint64_t data_size, byte_t** a)
{
  cbsl_ctx* ctx = cbsl_open(cbsl_store_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }
  for(uint64_t i = 0; i < num_vars; ++i)
    CBSL_ERROR_CHECK(cbsl_write(ctx, a[i], data_size));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void raw_write(uint64_t num_vars, uint64_t data_size, byte_t** a)
{
  FILE* fp = fopen(sname, "wb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen\n");
    exit(1);
  }
  for(int i = 0; i < num_vars; ++i)
    fwrite(a[i], 1, data_size, fp);
  fclose(fp);
}

void decompress(uint64_t num_vars, uint64_t data_size, byte_t** a)
{
  cbsl_ctx* ctx = cbsl_open(cbsl_load_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }
  for(uint64_t i = 0; i < num_vars; ++i)
    CBSL_ERROR_CHECK(cbsl_read(ctx, a[i], data_size));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void raw_read(uint64_t num_vars, uint64_t data_size, byte_t** a)
{
  FILE* fp = fopen(sname, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen\n");
    exit(1);
  }
  for(uint64_t i = 0; i < num_vars; ++i)
    fread(a[i], 1, data_size, fp);
  fclose(fp);
}

void rand_byte_t(uint64_t data_size, byte_t* a)
{
  for(uint64_t i = 0; i < data_size; ++i)
    a[i] = rand() % 255;
}
