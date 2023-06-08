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
extern void decompress(uint64_t* data_size, byte_t** a);
extern void raw_read(uint64_t* data_size, byte_t** a);

extern uint64_t rand_size();
extern void     rand_byte_t(uint64_t data_size, byte_t* a);

char cname[128], sname[128];

int main(int argc, char** argv)
{
  sprintf(cname, "varsize_compressed.zst");
  sprintf(sname, "varsize_raw.dat");

  srand((unsigned int)(time(NULL)));

  uint64_t data_size;
  if (argc >= 2)
  {
    sscanf(argv[1], "%lu\n", &data_size);
    printf("specified data size: %lu byte\n", data_size);
  }
  else
  {
    data_size = rand_size();
    printf("random generate data size: %lu byte\n", data_size);
  }

  byte_t* a = (byte_t*)(malloc(data_size));
  rand_byte_t(data_size / sizeof(byte_t), a);
  
  compress(data_size, a);
  raw_write(data_size, a);

  uint64_t b_data_size; byte_t* b;
  uint64_t c_data_size; byte_t* c;

  decompress(&b_data_size, &b);
  raw_read(&c_data_size, &c);

  if (data_size != b_data_size || b_data_size != c_data_size)
  {
    fprintf(stderr, "data size is mismatch!\n");
    exit(1);
  }

  for (uint64_t i = 0; i < data_size; ++i)
  {
    if (a[i] != b[i] || b[i] != c[i])
    {
      fprintf(stderr, "data value is mismatch!\n");
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
  CBSL_ERROR_CHECK(cbsl_write(ctx, &data_size, sizeof(data_size)));
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
  fwrite(&data_size, 1, sizeof(data_size), fp);
  fwrite(a, 1, data_size, fp);
  fclose(fp);
}

void decompress(uint64_t* data_size, byte_t** a)
{
  cbsl_ctx* ctx = cbsl_open(cbsl_load_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open(cbsl_load_mode)\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_read(ctx, data_size, sizeof(data_size)));
  *a = (byte_t*)(malloc(sizeof(byte_t) * *data_size));
  CBSL_ERROR_CHECK(cbsl_read(ctx, *a, *data_size));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}

void raw_read(uint64_t* data_size, byte_t** a)
{
  FILE* fp = fopen(sname, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen(rb)\n");
    exit(1);
  }
  fread(data_size, 1, sizeof(data_size), fp);
  *a = (byte_t*)(malloc(sizeof(byte_t) * *data_size));
  fread(*a, 1, *data_size, fp);
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
