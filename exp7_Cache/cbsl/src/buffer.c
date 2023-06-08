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

cbsl_errors create_streaming_buffers(cbsl_ctx* ctx)
{
	ZSTD_CCtx* zstd_cctx = NULL;
	ZSTD_DCtx* zstd_dctx = NULL;
	uint64_t in_buffer_size;
	uint64_t out_buffer_size;
	void* in_buffer = NULL;
	void* out_buffer = NULL;
	void* line_buffer = NULL;

	switch (ctx->mode)
	{
	case cbsl_load_mode:
		zstd_dctx = ZSTD_createDCtx();
		if (zstd_dctx == NULL) return cbsl_error;
		in_buffer_size = ZSTD_DStreamInSize();
		out_buffer_size = ZSTD_DStreamOutSize();
		break;
	case cbsl_store_mode:
		zstd_cctx = ZSTD_createCCtx();
		if (zstd_cctx == NULL) return cbsl_error;
		in_buffer_size = ZSTD_CStreamInSize();
		out_buffer_size = ZSTD_CStreamOutSize();
		break;
	default:
		return cbsl_error;
	}

	in_buffer = (byte_t*)(malloc(in_buffer_size));
	out_buffer = (byte_t*)(malloc(out_buffer_size));
	line_buffer = (byte_t*)(malloc(CBSL_LINEBUF_SIZE));

	if (in_buffer == NULL || out_buffer == NULL || line_buffer == NULL)
	{
		CBSL_SAFE_FREE_ZSTD_CCTX(zstd_cctx);
		CBSL_SAFE_FREE_ZSTD_DCTX(zstd_dctx);
		CBSL_SAFE_FREE(in_buffer);
		CBSL_SAFE_FREE(out_buffer);
		CBSL_SAFE_FREE(line_buffer);
		return cbsl_error;
	}

	ctx->zstd_cctx = zstd_cctx;
	ctx->zstd_dctx = zstd_dctx;
	ctx->in_buffer = in_buffer;
	ctx->out_buffer = out_buffer;
	ctx->line_buffer = line_buffer;

	ctx->in_buffer_size = in_buffer_size;
	ctx->out_buffer_size = out_buffer_size;

	ctx->in_buffer_pos = 0;
	ctx->out_buffer_pos = 0;

	ctx->in_buffer_used = 0;
	ctx->out_buffer_used = 0;
	ctx->line_buffer_used = 0;
	ctx->line_buffer_read_from_zst = 0;

	ctx->zstd_buf_end = 0;
	ctx->zstd_file_end = 0;

	return cbsl_success;
}
