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
#include "./cbsl_internal.h"

 /* FIXME: implicit declaration??? */
extern ZSTDLIB_API size_t ZSTD_CCtx_getParameter(ZSTD_CCtx* cctx, ZSTD_cParameter param, int* value);


cbsl_mode cbsl_get_mode(cbsl_ctx* ctx)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_unknown_mode);
	return ctx->mode;
}

cbsl_errors cbsl_set_compression_level(cbsl_ctx* ctx, int clevel)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(ctx->mode == cbsl_store_mode, cbsl_error);

	const size_t ret = ZSTD_CCtx_setParameter(ctx->zstd_cctx, ZSTD_c_compressionLevel, clevel);
	CBSL_CHECK_COND_MSG_AND_RETURN(!ZSTD_isError(ret), ZSTD_getErrorName(ret), cbsl_error);

	return cbsl_success;
}

int cbsl_get_compression_level(cbsl_ctx* ctx)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(ctx->mode == cbsl_store_mode, cbsl_error);

	int clevel;
	const size_t ret = ZSTD_CCtx_getParameter(ctx->zstd_cctx, ZSTD_c_compressionLevel, &clevel);
	CBSL_CHECK_COND_MSG_AND_RETURN(!ZSTD_isError(ret), ZSTD_getErrorName(ret), cbsl_error);

	return clevel;
}
