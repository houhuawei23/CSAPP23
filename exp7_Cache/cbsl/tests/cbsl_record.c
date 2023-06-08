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

extern void record(cbsl_mode, int*, double*, int*);

char cname[128];

int main(int argc, char** argv)
{
  sprintf(cname, "check_record_scalar.zst");

  int    c0 = data0;
  double c1 = data1;
  int    c2 = data2;
  record(cbsl_store_mode, &c0, &c1, &c2);

  c0 = c2 = 0;
  c1 = 0;
  record(cbsl_load_mode, &c0, &c1, &c2);

  CHECK_BINARY(c0, data0);
  CHECK_BINARY(c1, data1);
  CHECK_BINARY(c2, data2);

  return 0;
}

void record(cbsl_mode mode, int* c0, double* c1, int* c2)
{
  cbsl_ctx* ctx = cbsl_open(mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }
  CBSL_ERROR_CHECK(cbsl_record(ctx, c0, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_record(ctx, c1, sizeof(double)));
  CBSL_ERROR_CHECK(cbsl_record(ctx, c2, sizeof(int)));
  CBSL_ERROR_CHECK(cbsl_close(ctx));
}
