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
#ifndef CBSL_HEADER_INCLUDED
#define CBSL_HEADER_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "cbsl_config.h"

	typedef enum
	{
		cbsl_load_mode = 1,
		cbsl_store_mode = 2,
		cbsl_unknown_mode = -1
	}
	cbsl_mode;

	typedef enum
	{
		cbsl_success = 0,
		cbsl_end = 1,
		cbsl_error = -1
	}
	cbsl_errors;

	typedef struct cbsl_ctx_ cbsl_ctx;


	cbsl_ctx* cbsl_open(cbsl_mode open_mode, char* path);
	cbsl_errors    cbsl_close(cbsl_ctx* ctx);
	cbsl_errors    cbsl_flush(cbsl_ctx* ctx);

	cbsl_errors    cbsl_write(cbsl_ctx* ctx, const void* data, uint64_t size);
	cbsl_errors    cbsl_read(cbsl_ctx* ctx, void* data, uint64_t size);
	cbsl_errors    cbsl_readline(cbsl_ctx* ctx, char* linebuf, uint64_t size);
	cbsl_errors    cbsl_record(cbsl_ctx* ctx, void* data, uint64_t size);
	cbsl_errors    cbsl_record_heap(cbsl_ctx* ctx, void** data, uint64_t* size);

	cbsl_mode      cbsl_get_mode(cbsl_ctx* ctx);
	cbsl_errors    cbsl_set_compression_level(cbsl_ctx* ctx, int clevel);
	int            cbsl_get_compression_level(cbsl_ctx* ctx);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* CBSL_HEADER_INCLUDED */
