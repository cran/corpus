/*
 * Copyright 2017 Patrick O. Perry.
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

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "rcorpus.h"

#define CONTEXT_TAG install("corpus::context")


struct context {
	void *data;
	void (*destroy_func)(void *);
};


void free_context(SEXP x)
{
        struct context *ctx = R_ExternalPtrAddr(x);
        R_SetExternalPtrAddr(x, NULL);
	if (ctx) {
		if (ctx->destroy_func) {
			(ctx->destroy_func)(ctx->data);
		}
		corpus_free(ctx->data);
		corpus_free(ctx);
	}
}


SEXP alloc_context(size_t size, void (*destroy_func)(void *))
{
	SEXP ans;
	struct context *ctx = NULL;
	void *obj = NULL;
	int err = 0;

	PROTECT(ans = R_MakeExternalPtr(NULL, CONTEXT_TAG, R_NilValue));
        R_RegisterCFinalizerEx(ans, free_context, TRUE);

	TRY_ALLOC(obj = corpus_calloc(1, size == 0 ? 1 : size));
	TRY_ALLOC(ctx = corpus_calloc(1, sizeof(*ctx)));

	ctx->data = obj;
	ctx->destroy_func = destroy_func;
        R_SetExternalPtrAddr(ans, ctx);
	ctx = NULL;
	obj = NULL;
out:
	corpus_free(ctx);
	corpus_free(obj);
	CHECK_ERROR(err);
	UNPROTECT(1);
	return ans;
}


int is_context(SEXP x)
{
	return ((TYPEOF(x) == EXTPTRSXP)
                && (R_ExternalPtrTag(x) == CONTEXT_TAG));
}


void *as_context(SEXP x)
{
	struct context *ctx;

	if (!is_context(x)) {
		error("invalid context object");
	}

	ctx = R_ExternalPtrAddr(x);
	return ctx->data;
}
