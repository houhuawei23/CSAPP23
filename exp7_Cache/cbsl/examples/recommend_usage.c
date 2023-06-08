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
#define CHECK(X)             {if (!(X)) { fprintf(stderr, "error: %s\n", (#X)); }}
#define SAFE_FREE(X)         {if ((X) != NULL) { free((X)); }}

static const int    data0 = 43;
static const double data1 = 3.14159265;
static const int    data2 = 14142;

extern void checkpoint_restart(cbsl_mode, int*, double*, int*, int**);

char cname[128];

int main(int argc, char** argv)
{
  int    c0;
  double c1;
  int    c2;
  int*   a0;

  if (argc < 2)
    return 1;

  sprintf(cname, "checkpoint.zst");

  cbsl_mode mode = cbsl_unknown_mode;
  if (strcmp(argv[1],"-c") == 0)
  {
    mode = cbsl_store_mode;
    c0 = data0;
    c1 = data1;
    c2 = data2;
    a0 = (int*)(malloc(sizeof(int) * 42));
    for (int i = 0; i < 42; ++i)
      a0[i] = 42;
  }
  else if (strcmp(argv[1],"-r") == 0)
  {
    mode = cbsl_load_mode;
    c0 = c1 = c2 = 0;
    a0 = NULL;
  }
  else
  {
    return 1;
  }

  checkpoint_restart(mode, &c0, &c1, &c2, &a0);

  CHECK_BINARY(c0, data0);
  CHECK_BINARY(c1, data1);
  CHECK_BINARY(c2, data2);

  for (int i = 0; i < 42; ++i)
  {
    CHECK(a0[i] == 42);
  }

  SAFE_FREE(a0);

  return 0;
}

void checkpoint_restart(cbsl_mode mode, int* c0, double* c1, int* c2, int** a0)
{
  int a0_size;
  cbsl_ctx* ctx = cbsl_open(mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_record(ctx, c0, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_record(ctx, c1, sizeof(double)));
  CBSL_ERROR_CHECK(cbsl_record(ctx, c2, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_record(ctx, &a0_size, sizeof(int)));
  if (mode == cbsl_load_mode)
    (*a0) = (int*)(malloc(sizeof(int) * a0_size));
  CBSL_ERROR_CHECK(cbsl_record(ctx, (*a0), sizeof(int) * a0_size));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}
