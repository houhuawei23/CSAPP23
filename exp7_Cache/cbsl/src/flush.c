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
cbsl_errors flush_serialize(cbsl_ctx* ctx)
{
	if (ctx->in_buffer_used > 0)
	{
		ZSTD_inBuffer input = { ctx->in_buffer, ctx->in_buffer_used, 0 };
		ZSTD_EndDirective mode = (ctx->in_buffer_used < ctx->in_buffer_size) ? ZSTD_e_end : ZSTD_e_continue;
		do
		{
			ZSTD_outBuffer output = { ctx->out_buffer, ctx->out_buffer_size, 0 };
			const size_t remaining = ZSTD_compressStream2(ctx->zstd_cctx, &output, &input, mode);
			CBSL_CHECK_COND_MSG_AND_RETURN(!ZSTD_isError(remaining), ZSTD_getErrorName(remaining), cbsl_error);

			const size_t written = fwrite(ctx->out_buffer, 1, output.pos, ctx->fp);
			CBSL_CHECK_COND_AND_RETURN(written == output.pos, cbsl_error);
		} while (input.pos < input.size);

		ctx->in_buffer_used = 0;
	}

	fflush(ctx->fp);

	return cbsl_success;
}

cbsl_errors cbsl_flush(cbsl_ctx* ctx)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	if (ctx->mode == cbsl_store_mode)
	{
		return flush_serialize(ctx);
	}
	return cbsl_success;
}
