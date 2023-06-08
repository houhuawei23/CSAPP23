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
#include <stdint.h>
#include <stdlib.h>

#include "./cbsl_internal.h"

cbsl_errors cbsl_record(cbsl_ctx* ctx, void* data, uint64_t size)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(data != NULL, cbsl_error);

	switch (ctx->mode)
	{
	case cbsl_load_mode:  return cbsl_read(ctx, data, size);
	case cbsl_store_mode: return cbsl_write(ctx, data, size);
	default:              return cbsl_error;
	}
}

cbsl_errors cbsl_record_heap(cbsl_ctx* ctx, void** data, uint64_t* size)
{
	CBSL_CHECK_COND_AND_RETURN(ctx != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(data != NULL, cbsl_error);
	CBSL_CHECK_COND_AND_RETURN(size != NULL, cbsl_error);

	switch (ctx->mode)
	{
	case cbsl_load_mode:
	{
		uint64_t rsize;
		void* rdata = *data;
		CBSL_CHECK_COND_AND_RETURN(cbsl_read(ctx, &rsize, sizeof(rsize)) == cbsl_success, cbsl_error);
		if (rdata == NULL)
		{
			CBSL_CHECK_COND_AND_RETURN((rdata = malloc(rsize)) != NULL, cbsl_error);
		}
		else
		{
			CBSL_CHECK_COND_AND_RETURN(rsize == *size, cbsl_error);
		}
		CBSL_CHECK_COND_AND_RETURN(cbsl_read(ctx, rdata, rsize) == cbsl_success, cbsl_error);
		*size = rsize;
		*data = rdata;
	}
	break;
	case cbsl_store_mode:
	{
		CBSL_CHECK_COND_AND_RETURN(cbsl_write(ctx, size, sizeof(*size)) == cbsl_success, cbsl_error);
		CBSL_CHECK_COND_AND_RETURN(cbsl_write(ctx, *data, *size) == cbsl_success, cbsl_error);
	}
	break;
	default:
		return cbsl_error;
	}
	return cbsl_success;
}
