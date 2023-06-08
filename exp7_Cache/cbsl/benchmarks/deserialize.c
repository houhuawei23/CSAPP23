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
#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif
#include <math.h>
#include <cbsl.h>

#include "config.h"

#define CBSL_ERROR_CHECK(X)  {if ((X) == cbsl_error) { fprintf(stderr, "error: %s\n", (#X)); exit(1); }}

typedef unsigned char byte_t;

extern void deserialize_bench(double ds);
extern double seconds();

int main(int argc, char** argv)
{
  if (argc <= 2)
    exit(1);

  int max_data_size_exp2, min_data_size_exp2;
  sscanf(argv[1], "%d", &min_data_size_exp2);
  sscanf(argv[2], "%d", &max_data_size_exp2);
  printf("min data size = %.2lf [B]\n", pow(2,min_data_size_exp2));
  printf("max data size = %.2lf [B]\n", pow(2,max_data_size_exp2));

  srand((unsigned int)(time(NULL)));

  printf("<read data [Byte]> <time [seconds]> <speed [MiB/sec]>\n");

  for (int i = min_data_size_exp2; i <= max_data_size_exp2; ++i)
  {
    deserialize_bench(pow(2,i));
  }

  return 0;
}

void deserialize_bench(double ds)
{
  const uint64_t data_size = (uint64_t)(ds);

  char cname[128];
  sprintf(cname, "serialize_%lu.zst", data_size);

  byte_t* a = (byte_t*)(malloc(data_size));

  cbsl_ctx* ctx = cbsl_open(cbsl_load_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }

  const double beg = seconds();
  {
    CBSL_ERROR_CHECK(cbsl_read(ctx, a, data_size));
  }
  const double end = seconds();
  const double rt  = end - beg;

  CBSL_ERROR_CHECK(cbsl_close(ctx));

  printf("%lu    %lf    %lf\n", data_size, rt, (data_size/rt)/1.0e6);

  free(a);
}

double seconds() {
#if defined(HAVE_CLOCK_GETTIME)
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return ts.tv_sec + (ts.tv_nsec / 1.0e9);
#elif defined(HAVE_GETTIMEOFDAY)
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (ts.tv_usec / 1.0e6);
#else
  return (double)(clock() / CLOCKS_PER_SEC);
#endif
}
