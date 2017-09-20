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

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rcorpus.h"

struct context {
	struct corpus_typemap map;
	int has_map;
};


static void context_init(struct context *ctx, SEXP map_case, SEXP map_compat,
			 SEXP map_quote, SEXP remove_ignorable)
{
	int err = 0, kind;

	kind = CORPUS_TYPE_NORMAL;

	if (LOGICAL(map_case)[0] == TRUE) {
		kind |= CORPUS_TYPE_MAPCASE;
	}

	if (LOGICAL(map_compat)[0] == TRUE) {
		kind |= CORPUS_TYPE_MAPCOMPAT;
	}

	if (LOGICAL(map_quote)[0] == TRUE) {
		kind |= CORPUS_TYPE_MAPQUOTE;
	}

	if (LOGICAL(remove_ignorable)[0] == TRUE) {
		kind |= CORPUS_TYPE_RMDI;
	}

	TRY(corpus_typemap_init(&ctx->map, kind, NULL));
	ctx->has_map = 1;
out:
	CHECK_ERROR(err);
}


static void context_destroy(void *obj)
{
	struct context *ctx = obj;

	if (ctx->has_map) {
		corpus_typemap_destroy(&ctx->map);
	}
}


SEXP utf8_normalize(SEXP x, SEXP map_case, SEXP map_compat, SEXP map_quote,
		    SEXP remove_ignorable)
{
	SEXP ans, sctx, elt;
	struct context *ctx;
	struct corpus_text text;
	const uint8_t *ptr;
	size_t size;
	R_xlen_t i, n;
	int err = 0, nprot = 0;

	if (x == R_NilValue) {
		return R_NilValue;
	}

	PROTECT(sctx = alloc_context(sizeof(*ctx), context_destroy)); nprot++;
        ctx = as_context(sctx);
	context_init(ctx, map_case, map_compat, map_quote, remove_ignorable);

	PROTECT(ans = duplicate(x)); nprot++;
	n = XLENGTH(ans);

	for (i = 0; i < n; i++) {
		RCORPUS_CHECK_INTERRUPT(i);

		elt = STRING_ELT(ans, i);
		if (elt == NA_STRING) {
			continue;
		}

		ptr = (const uint8_t *)translate_utf8(elt);
		size = strlen((const char *)ptr);
		TRY(corpus_text_assign(&text, ptr, size, CORPUS_TEXT_NOESCAPE));
		TRY(corpus_typemap_set(&ctx->map, &text));

		ptr = ctx->map.type.ptr;
		size = CORPUS_TEXT_SIZE(&ctx->map.type);
		TRY(size > INT_MAX ? CORPUS_ERROR_OVERFLOW : 0);

		elt = mkCharLenCE((const char *)ptr, (int)size, CE_UTF8);
		SET_STRING_ELT(ans, i, elt);
	}

out:
	CHECK_ERROR(err);
	free_context(sctx);
	UNPROTECT(nprot);
	return ans;
}
