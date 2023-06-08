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

static const int    data0 = 43;
static const double data1 = 3.14159265;
static const int    data2 = 14142;

extern void compress();
extern void decompress();

char cname[128], sname[128];

int main(int argc, char** argv)
{
  if (argc < 2)
    return 1;

  sprintf(cname, "scalar_compressed.zst");
  sprintf(sname, "scalar_raw.dat");

  if (strcmp(argv[1],"-c") == 0)
    compress();
  else if (strcmp(argv[1],"-d") == 0)
    decompress();
  else
    return 1;

  return 0;
}

void compress()
{
  const int    c0 = data0;
  const double c1 = data1;
  const int    c2 = data2;

  cbsl_ctx* ctx = cbsl_open(cbsl_store_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_write(ctx, &c0, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_write(ctx, &c1, sizeof(double)));
  CBSL_ERROR_CHECK(cbsl_write(ctx, &c2, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_close(ctx));

  FILE* fp = fopen(sname, "wb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen\n");
    exit(1);
  }
  fwrite(&c0, sizeof(int),    1, fp);
  fwrite(&c1, sizeof(double), 1, fp);
  fwrite(&c2, sizeof(int),    1, fp);
  fclose(fp);
}

void decompress()
{
  int    d0, r0;
  double d1, r1;
  int    d2, r2;

  cbsl_ctx* ctx = cbsl_open(cbsl_load_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_read(ctx, &d0, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_read(ctx, &d1, sizeof(double)));
  CBSL_ERROR_CHECK(cbsl_read(ctx, &d2, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_close(ctx));

  FILE* fp = fopen(sname, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "error: fopen\n");
    exit(1);
  }
  fread(&r0, sizeof(int),    1, fp);
  fread(&r1, sizeof(double), 1, fp);
  fread(&r2, sizeof(int),    1, fp);
  fclose(fp);

  CHECK_BINARY(d0, r0); CHECK_BINARY(d0, data0);
  CHECK_BINARY(d1, r1); CHECK_BINARY(d1, data1);
  CHECK_BINARY(d2, r2); CHECK_BINARY(d2, data2);
}
