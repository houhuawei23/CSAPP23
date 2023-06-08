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

extern void compress(uint64_t data_size, const byte_t* a);
extern void raw_write(uint64_t data_size, const byte_t* a);
extern void decompress(uint64_t data_size, byte_t* a);
extern void raw_read(uint64_t data_size, byte_t* a);

extern void rand_byte_t(uint64_t data_size, byte_t* a);

char cname[128], sname[128];

int main(int argc, char** argv)
{
  if (argc < 2)
    exit(1);

  int data_size;
  sscanf(argv[1], "%d", &data_size);
  printf("data size = %lf [MiB]\n", (double)(data_size)/pow(2,20));

  sprintf(cname, "simple_compressed_%d.zst", data_size);
  sprintf(sname, "simple_raw_%d.dat", data_size);

  srand((unsigned int)(time(NULL)));

  byte_t* a = (byte_t*)(malloc(data_size));
  rand_byte_t(data_size / sizeof(byte_t), a);
  
  compress(data_size, a);
  raw_write(data_size, a);

  byte_t* b = (byte_t*)(malloc(data_size));
  byte_t* c = (byte_t*)(malloc(data_size));

  decompress(data_size, b);
  raw_read(data_size, c);

  for (uint64_t i = 0; i < data_size; ++i)
  {
    if (a[i] != b[i])
    {
      fprintf(stderr, "mismatch!\n");
      exit(1);
    }
  }

  free(a);
  free(b);
  free(c);

  return 0;
}

void compress(uint64_t data_size, const byte_t* a)
{
  cbsl_ctx* ctx = cbsl_open(cbsl_store_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open(cbsl_store_mode)\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_write(ctx, a, data_size));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void raw_write(uint64_t data_size, const byte_t* a)
{
  FILE* fp = fopen(sname, "wb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen(wb)\n");
    exit(1);
  }
  fwrite(a, 1, data_size, fp);
  fclose(fp);
}

void decompress(uint64_t data_size, byte_t* a)
{
  cbsl_ctx* ctx = cbsl_open(cbsl_load_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open(cbsl_load_mode)\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_read(ctx, a, data_size));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void raw_read(uint64_t data_size, byte_t* a)
{
  FILE* fp = fopen(sname, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen(rb)\n");
    exit(1);
  }
  fread(a, 1, data_size, fp);
  fclose(fp);
}

void rand_byte_t(uint64_t data_size, byte_t* a)
{
  for(uint64_t i = 0; i < data_size; ++i)
    a[i] = rand() % 255;
}
