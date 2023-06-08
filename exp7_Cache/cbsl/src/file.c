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
#include <stdint.h>

#include "./cbsl_internal.h"

static
cbsl_ctx* cbsl_open_safe_finalize(cbsl_ctx** ctx)
{
	if (*ctx != NULL)
	{
		cbsl_close(*ctx);
	}
	return NULL;
}

cbsl_ctx* cbsl_open(cbsl_mode mode, char* path)
{
	CBSL_CHECK_COND_AND_RETURN(path != NULL, NULL);

	cbsl_ctx* ctx = (cbsl_ctx*)(malloc(sizeof(cbsl_ctx)));
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, NULL);

	char const* fopen_mode;
	switch (mode)
	{
	case cbsl_load_mode:  fopen_mode = "rb"; break;
	case cbsl_store_mode: fopen_mode = "wb"; break;
	default:              free(ctx);         return NULL;
	}

#ifdef _WIN32
	errno_t err;
	FILE* fp;
	err = fopen_s(&fp, path, fopen_mode);
	CBSL_CHECK_COND_AND_RETURN(err == 0, cbsl_open_safe_finalize(&ctx));
#else
	FILE* fp = fopen(path, fopen_mode);
	CBSL_CHECK_COND_AND_RETURN(fp != NULL, cbsl_open_safe_finalize(&ctx));
#endif

	ctx->mode = mode;
	ctx->fp = fp;
	CBSL_CHECK_COND_AND_RETURN(create_streaming_buffers(ctx) == cbsl_success, cbsl_open_safe_finalize(&ctx));

	/*
	uint64_t file_version;
	if (mode == cbsl_load_mode)
	{
	  file_version = 0;
	  CBSL_CHECK_COND_AND_RETURN(cbsl_read(ctx, &file_version, sizeof(file_version)) == cbsl_success, cbsl_open_safe_finalize(&ctx));
	  CBSL_CHECK_COND_AND_RETURN(file_version == CBSL_VERSION, cbsl_open_safe_finalize(&ctx));
	}
	else if (mode == cbsl_store_mode)
	{
	  file_version = CBSL_VERSION;
	  CBSL_CHECK_COND_AND_RETURN(cbsl_write(ctx, &file_version, sizeof(file_version)) == cbsl_success, cbsl_open_safe_finalize(&ctx));
	}
	*/

	return ctx;
}

cbsl_errors cbsl_close(cbsl_ctx* ctx)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_success);
	if (ctx->mode == cbsl_store_mode)
	{
		cbsl_flush(ctx);
	}
	CBSL_SAFE_FREE_ZSTD_CCTX(ctx->zstd_cctx);
	CBSL_SAFE_FREE_ZSTD_DCTX(ctx->zstd_dctx);
	CBSL_SAFE_FREE(ctx->in_buffer);
	CBSL_SAFE_FREE(ctx->out_buffer);
	CBSL_SAFE_FREE(ctx->line_buffer);
	CBSL_SAFE_FCLOSE(ctx->fp);
	CBSL_SAFE_FREE(ctx);
	return cbsl_success;
}
