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
#include <string.h>

#include "./cbsl_internal.h"

static
cbsl_errors streaming_write_flush(cbsl_ctx* ctx, uint64_t size)
{
	if (ctx->in_buffer_used > 0 && ctx->in_buffer_used + size > ctx->in_buffer_size)
	{
		CBSL_CHECK_COND_AND_RETURN(cbsl_flush(ctx) == cbsl_success, cbsl_error);
	}
	return cbsl_success;
}

static
cbsl_errors streaming_write_immediate(cbsl_ctx* ctx, const void* data, uint64_t size)
{
	CBSL_DEBUG_MESSAGE("%s: call\n", __func__);
	CBSL_CHECK_COND_AND_RETURN(streaming_write_flush(ctx, size) == cbsl_success, cbsl_error);

	uint64_t written = 0;
	const byte_t* pdata = (const byte_t*)(data);
	while (written < size)
	{
		/* compressing data copies to internal input buffer */
		const uint64_t write_size = MIN(size - written, ctx->in_buffer_size);
		memcpy(ctx->in_buffer, pdata + written, write_size);
		written += write_size;

		ZSTD_inBuffer input = { ctx->in_buffer, write_size, 0 };
		ZSTD_EndDirective mode = (size - written < ctx->in_buffer_size) ? ZSTD_e_end : ZSTD_e_continue;
		do
		{
			ZSTD_outBuffer output = { ctx->out_buffer, ctx->out_buffer_size, 0 };
			const size_t remaining = ZSTD_compressStream2(ctx->zstd_cctx, &output, &input, mode);
			CBSL_CHECK_COND_MSG_AND_RETURN(!ZSTD_isError(remaining), ZSTD_getErrorName(remaining), cbsl_error);
			CBSL_CHECK_COND_AND_RETURN(fwrite(ctx->out_buffer, 1, output.pos, ctx->fp) == output.pos, cbsl_error);
		} while (input.pos < input.size);
		CBSL_ASSERT(input.pos == input.size);
	}
	CBSL_ASSERT(written == size);

	return cbsl_success;
}

static
cbsl_errors streaming_write_buffered(cbsl_ctx* ctx, const void* data, uint64_t size)
{
	CBSL_DEBUG_MESSAGE("%s: call\n", __func__);
	CBSL_CHECK_COND_AND_RETURN(streaming_write_flush(ctx, size) == cbsl_success, cbsl_error);

	uint64_t written = 0;
	const byte_t* pdata = (const byte_t*)(data);
	do
	{
		const uint64_t write_size = MIN(size - written, ctx->in_buffer_size);
		memcpy(ctx->in_buffer, pdata + written, write_size);
		ctx->in_buffer_used = write_size;
		CBSL_CHECK_COND_AND_RETURN(cbsl_flush(ctx) == cbsl_success, cbsl_error);
		written += write_size;
	} while (written < size);
	CBSL_ASSERT(written == size);

	return cbsl_success;
}

cbsl_errors cbsl_write(cbsl_ctx* ctx, const void* data, uint64_t size)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(ctx->mode == cbsl_store_mode, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(data != NULL, cbsl_error);

	CBSL_DEBUG_MESSAGE("write: %lu bytes\n", size);
	if (size > ctx->in_buffer_size)
	{
		/* data size is larger than compression buffer */
		return streaming_write_immediate(ctx, data, size);
	}
	else
	{
		return streaming_write_buffered(ctx, data, size);
	}
}
