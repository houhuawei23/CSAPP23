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
#ifndef CBSL_INTERNAL_HEADER_INCLUDED
#define CBSL_INTERNAL_HEADER_INCLUDED

#include <stdio.h>
#include <stdint.h>
#include <zstd.h>

#include <cbsl.h>

#define CBSL_LINEBUF_SIZE	8192

typedef unsigned char byte_t;

struct cbsl_ctx_
{
	FILE* fp;
	ZSTD_CCtx* zstd_cctx;
	ZSTD_DCtx* zstd_dctx;
	byte_t* in_buffer;
	byte_t* out_buffer;
	uint64_t      in_buffer_size;
	uint64_t      out_buffer_size;
	uint64_t      in_buffer_pos;
	uint64_t      in_buffer_used;
	uint64_t      out_buffer_pos;
	uint64_t      out_buffer_used;
	cbsl_mode     mode;

	// support readline
	byte_t		zstd_buf_end;
	byte_t		zstd_file_end;
	byte_t* line_buffer;
	uint64_t      line_buffer_used;
	uint64_t		line_buffer_read_from_zst;
};

cbsl_errors create_streaming_buffers(cbsl_ctx* ctx);

#define MIN(x, y) (x) < (y) ? (x) : (y)

#ifdef CBSL_DEBUG
# include <assert.h>
# define CBSL_ASSERT(COND)       { assert((COND)); }
# define CBSL_DEBUG_MESSAGE(...) { fprintf(stderr, __VA_ARGS__); }
#else
# define CBSL_ASSERT(COND)       /* */
# define CBSL_DEBUG_MESSAGE(...) /* */
#endif

/* error check */
#define CBSL_CHECK_COND_MSG_IMPL(X,MSG,FINALIZE)      { if (!(X)) { fprintf(stderr, "%s at %s l.%d: condition error %s; %s\n", __func__, __FILE__, __LINE__, (#X), (MSG)); FINALIZE; } }
#define CBSL_CHECK_COND(X)                            CBSL_CHECK_COND_MSG_IMPL((X), "", /* noreturn */)
#define CBSL_CHECK_COND_AND_RETURN(X,RETCODE)         CBSL_CHECK_COND_MSG_IMPL((X), "", { return (RETCODE); })
#define CBSL_CHECK_COND_MSG_AND_RETURN(X,MSG,RETCODE) CBSL_CHECK_COND_MSG_IMPL((X), (MSG), { return (RETCODE); })

/* safe memory control */
#define CBSL_SAFE_FREE_RESOURCE(X,FREEFUNC) { if ((X) != NULL) { (FREEFUNC)((X)); (X) = NULL; } }
#define CBSL_SAFE_FCLOSE(X)                 CBSL_SAFE_FREE_RESOURCE((X), fclose);
#define CBSL_SAFE_FREE(X)                   CBSL_SAFE_FREE_RESOURCE((X), free)
#define CBSL_SAFE_FREE_ZSTD_CCTX(X)         CBSL_SAFE_FREE_RESOURCE((X), ZSTD_freeCCtx)
#define CBSL_SAFE_FREE_ZSTD_DCTX(X)         CBSL_SAFE_FREE_RESOURCE((X), ZSTD_freeDCtx)

#endif /* CBSL_INTERNAL_HEADER_INCLUDED */
