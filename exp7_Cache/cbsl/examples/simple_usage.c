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
#include <string.h>
#include <cbsl.h>

#define ARRAY_SIZE(X)        (sizeof((X))/sizeof(*(X)))
#define CBSL_ERROR_CHECK(X)  {if ((X) == cbsl_error) { fprintf(stderr, "error: %s\n", (#X)); }}

int    a[1024]; /* 4 KiB */
double b[1024]; /* 8 KiB */

extern void save(void);
extern void load(void);
extern void rand_int(int, int*);
extern void rand_double(int, double*);

char cname[128];

int main(int argc, char** argv) {
  srand((unsigned int)(time(NULL)));

  sprintf(cname, "checkpoint.zst");

  if (strcmp(argv[1],"-c") == 0)
  {
    rand_int(ARRAY_SIZE(a), a);
    rand_double(ARRAY_SIZE(b), b);
    save();
  }
  else if (strcmp(argv[1],"-r") == 0)
  {
    load();
  }
  else
  {
    return 1;
  }

  return 0;
}

void save(void) {
  cbsl_ctx* ctx = cbsl_open(cbsl_store_mode, "./checkpoint.data");

  if (ctx == NULL) {
    fprintf(stderr, "error: cbsl_open save\n");
    exit(1);
  }

  CBSL_ERROR_CHECK(cbsl_write(ctx, a, sizeof(a)));
  CBSL_ERROR_CHECK(cbsl_write(ctx, b, sizeof(b)));
  CBSL_ERROR_CHECK(cbsl_close(ctx));

  printf("before compressed a[%d] = %d\n", 124, a[124]);
  printf("before compressed b[%d] = %e\n", 514, b[514]);
}

void load(void) {
  cbsl_ctx* ctx = cbsl_open(cbsl_load_mode, "./checkpoint.data");

  if (ctx == NULL) {
    fprintf(stderr, "error: cbsl_open load\n");
    exit(1);
  }

  CBSL_ERROR_CHECK(cbsl_read(ctx, a, sizeof(a)));
  CBSL_ERROR_CHECK(cbsl_read(ctx, b, sizeof(b)));
  CBSL_ERROR_CHECK(cbsl_close(ctx));

  printf("decompressed a[%d] = %d\n", 124, a[124]);
  printf("decompressed b[%d] = %e\n", 514, b[514]);
}


void rand_int(int n, int *v) {
  for(int i = 0; i < n; ++i)
    v[i] = rand();
}

void rand_double(int n, double *v) {
  for(int i = 0; i < n; ++i)
    v[i] = 1.0 / (double)(rand());
}
