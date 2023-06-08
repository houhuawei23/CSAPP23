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
#define CHECK(X)             {if (!(X)) { fprintf(stderr, "error: %s\n", (#X)); exit(1); }}

char cname[128];

int main(int argc, char** argv)
{
  sprintf(cname, "data.zst");
  cbsl_ctx* ctx = cbsl_open(cbsl_store_mode, cname);
  if (ctx == NULL)
  {
    fprintf(stderr, "error: cbsl_open\n");
    exit(1);
  }

  CHECK(cbsl_get_mode(ctx) == cbsl_store_mode);
  CBSL_ERROR_CHECK(cbsl_set_compression_level(ctx, 20));
  CHECK(cbsl_get_compression_level(ctx) == 20);

  CBSL_ERROR_CHECK(cbsl_close(ctx));

  return 0;
}
