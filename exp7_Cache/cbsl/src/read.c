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
#include <zstd.h>

#include "./cbsl_internal.h"

static
cbsl_errors streaming_read_immediate(cbsl_ctx* ctx, void* data, uint64_t size)
{
	CBSL_DEBUG_MESSAGE("%s: call\n", __func__);

	/* previous all decompressed data loaded */
	CBSL_CHECK_COND_AND_RETURN(ctx->out_buffer_pos >= ctx->out_buffer_used, cbsl_error);

	ZSTD_outBuffer output = { data, size, 0 }; /* decompressed data stores directly */
	while (output.pos < output.size)
	{
		/* previous read data from file */
		if (ctx->in_buffer_pos < ctx->in_buffer_used)
		{
			/* input buffer has remained data from the previous decompression */
			memmove(ctx->in_buffer, ctx->in_buffer + ctx->in_buffer_pos, ctx->in_buffer_used - ctx->in_buffer_pos);
			ctx->in_buffer_used -= ctx->in_buffer_pos;
		}
		else
		{
			ctx->in_buffer_used = 0ULL;
		}
		ctx->in_buffer_pos = 0ULL;

		ctx->in_buffer_used += fread(ctx->in_buffer + ctx->in_buffer_used, 1, ctx->in_buffer_size - ctx->in_buffer_used, ctx->fp);

		ZSTD_inBuffer input = { ctx->in_buffer, ctx->in_buffer_used, ctx->in_buffer_pos };
		while (input.pos < input.size && output.pos < output.size)
		{
			const size_t ret = ZSTD_decompressStream(ctx->zstd_dctx, &output, &input);
			CBSL_CHECK_COND_MSG_AND_RETURN(!ZSTD_isError(ret), ZSTD_getErrorName(ret), cbsl_error);
		}
		ctx->in_buffer_used = input.size;
		ctx->in_buffer_pos = input.pos;
	}
	CBSL_ASSERT(output.pos == output.size);
	ctx->out_buffer_pos = output.pos;
	ctx->out_buffer_used = output.size;

	return cbsl_success;
}

static
uint64_t read_from_buffer(cbsl_ctx* ctx, byte_t* data, uint64_t size)
{
	uint64_t read = 0;
	if (ctx->out_buffer_pos < ctx->out_buffer_used)
	{
		const uint64_t read_size = MIN(ctx->out_buffer_used - ctx->out_buffer_pos, size);
		memcpy(data, ctx->out_buffer + ctx->out_buffer_pos, read_size);
		ctx->out_buffer_pos += read_size;
		read = read_size;
	}
	return read;
}

static
cbsl_errors streaming_read_buffered(cbsl_ctx* ctx, void* data, uint64_t size)
{
	CBSL_DEBUG_MESSAGE("%s: call\n", __func__);

	size_t zstd_ret = -1;

	/* memcpy from the previous decompression data */
	uint64_t read = read_from_buffer(ctx, data, size);

	byte_t* pdata = (byte_t*)(data);

	/* streaming decompression and memcpy */
	while (read < size)
	{
		if (ctx->in_buffer_pos < ctx->in_buffer_used)
		{
			/* input buffer has remained data from the previous decompression */
			memmove(ctx->in_buffer, ctx->in_buffer + ctx->in_buffer_pos, ctx->in_buffer_used - ctx->in_buffer_pos);
			ctx->in_buffer_used -= ctx->in_buffer_pos;
		}
		else
		{
			/* input buffer is already consumed by the previous decompression */
			ctx->in_buffer_used = 0;
		}
		ctx->in_buffer_pos = 0ULL;

		ctx->in_buffer_used += fread(ctx->in_buffer + ctx->in_buffer_used, 1, ctx->in_buffer_size - ctx->in_buffer_used, ctx->fp);

		if (ctx->in_buffer_used == 0)   // 从文件中，读取不到数据了，无法解压
			break;
		ZSTD_inBuffer input = { ctx->in_buffer, ctx->in_buffer_used, ctx->in_buffer_pos };
		while (input.pos < input.size && read < size)
		{
			ZSTD_outBuffer output = { ctx->out_buffer, ctx->out_buffer_size, 0 };
			zstd_ret = ZSTD_decompressStream(ctx->zstd_dctx, &output, &input);
			CBSL_CHECK_COND_MSG_AND_RETURN(!ZSTD_isError(zstd_ret), ZSTD_getErrorName(zstd_ret), cbsl_error);
			ctx->out_buffer_pos = 0ULL;
			ctx->out_buffer_used = output.pos;
			read += read_from_buffer(ctx, pdata + read, size - read);
		}
		ctx->in_buffer_pos = input.pos;
		ctx->in_buffer_used = input.size;

		if (zstd_ret == 0)
		{
			// zstd stream end
			ctx->zstd_file_end = 1;
			break;
		}
	}
	if (ctx->zstd_file_end == 1 && ctx->out_buffer_pos == ctx->out_buffer_used)
		ctx->zstd_buf_end = 1;
	CBSL_ASSERT(read == size || ctx->zstd_end == 1);

	ctx->line_buffer_read_from_zst = read;

	return cbsl_success;
}

cbsl_errors cbsl_read(cbsl_ctx* ctx, void* data, uint64_t size)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(ctx->mode == cbsl_load_mode, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(data != NULL, cbsl_error);

	CBSL_DEBUG_MESSAGE("read: %lu bytes\n", size);

	if (size > ctx->out_buffer_size)
	{
		/* data size is larger than compression buffer */
		return streaming_read_immediate(ctx, data, size);
	}
	else
	{
		return streaming_read_buffered(ctx, data, size);
	}
}

// readline from stream
cbsl_errors cbsl_readline(cbsl_ctx* ctx, char* data, uint64_t size)
{
	cbsl_errors ret = cbsl_success;
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(ctx->mode == cbsl_load_mode, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(data != NULL, cbsl_error);

	CBSL_DEBUG_MESSAGE("%s: %lu bytes\n", __func__, size);
	uint64_t line_buffer_space = 0;
	line_buffer_space = CBSL_LINEBUF_SIZE - ctx->line_buffer_used;

	if (ctx->zstd_buf_end == 0)
	{
		if (line_buffer_space > ctx->out_buffer_size)
		{
			/* line_buffer size is larger than compression buffer */
			ret = streaming_read_immediate(ctx, ctx->line_buffer + ctx->line_buffer_used, line_buffer_space);
		}
		else
		{
			ret = streaming_read_buffered(ctx, ctx->line_buffer + ctx->line_buffer_used, line_buffer_space);
		}
		ctx->line_buffer_used += ctx->line_buffer_read_from_zst;
	}

	uint64_t i;
	char* ptr;
	ptr = (char*)ctx->line_buffer;
	data[0] = '\0';
	for (i = 0; i < ctx->line_buffer_used; i++)
	{
		if (*ptr == '\n' || *ptr == '\r')
		{
			CBSL_CHECK_COND_AND_RETURN(size >= i - 1, cbsl_error);
			memcpy(data, ctx->line_buffer, i + 1);
			memmove(ctx->line_buffer, ctx->line_buffer + i + 1, ctx->line_buffer_used - i - 1);
			ctx->line_buffer_used -= i + 1;
			data[i + 1] = '\0';
			break;
		}
		ptr++;
	}

	if (ctx->line_buffer_used == 0)
		return cbsl_end;
	else
		return ret;
}