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

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "corpus/src/error.h"
#include "corpus/src/memory.h"
#include "corpus/src/render.h"
#include "corpus/src/table.h"
#include "corpus/src/tree.h"
#include "corpus/src/termset.h"
#include "corpus/src/text.h"
#include "corpus/src/textset.h"
#include "corpus/src/typemap.h"
#include "corpus/src/symtab.h"
#include "corpus/src/wordscan.h"
#include "corpus/src/filter.h"
#include "corpus/src/ngram.h"
#include "rcorpus.h"


// the R 'error' is a #define (to Rf_error) that clashes with the 'error'
// member of struct corpus_filter
#ifdef error
#  undef error
#endif


#define CLEANUP() \
	do { \
		if (has_render) { \
			corpus_render_destroy(&render); \
			has_render = 0; \
		} \
		if (has_termset) { \
			corpus_termset_destroy(&termset); \
			has_termset = 0; \
		} \
		while (has_ngram-- > 0) { \
			corpus_ngram_destroy(&ngram[has_ngram]); \
		} \
		free(ngram); \
		ngram = NULL; \
	} while (0)


SEXP term_matrix_text(SEXP sx, SEXP sprops, SEXP sweights, SEXP sngrams,
		      SEXP sselect, SEXP sgroup)
{
	SEXP ans = R_NilValue, snames, si, sj, scount, stext, sfilter,
	     scol_names, srow_names, sterm;
	const struct corpus_text *text, *type;
	struct corpus_render render;
	struct corpus_filter *filter;
	struct termset *select;
	struct corpus_termset termset;
	const struct corpus_termset *terms;
	const double *weights;
	const int *ngrams, *type_ids;
	int *buffer, *ngram_set;
	const int *group;
	double w;
	struct corpus_ngram *ngram;
	struct corpus_ngram_iter it;
	R_xlen_t i, n, g, ngroup, nz, off;
	int err, j, m, ngram_max, term_id, type_id, nprot = 0;
	int has_render, has_termset, has_ngram;

	has_render = 0;
	has_ngram = 0;
	has_termset = 0;
	ngram = NULL;

	PROTECT(stext = coerce_text(sx)); nprot++;
	text = as_text(stext, &n);

	PROTECT(sfilter = alloc_filter(sprops)); nprot++;
	filter = as_filter(sfilter);

	if (sselect != R_NilValue) {
		PROTECT(sselect = alloc_termset(sselect, "select", filter, 0));
		nprot++;
		select = as_termset(sselect);
	} else {
		select = NULL;
	}

	if (sngrams != R_NilValue) {
		PROTECT(sngrams = coerceVector(sngrams, INTSXP)); nprot++;
		ngrams = INTEGER(sngrams);
		ngram_max = 1;
		for (i = 0; i < XLENGTH(sngrams); i++) {
			if (ngrams[i] == NA_INTEGER) {
				continue;
			}
			if (ngrams[i] > ngram_max) {
				ngram_max = ngrams[i];
			}
		}
	} else {
		ngrams = NULL;
		ngram_max = select ? select->max_length : 1;
	}

	buffer = (void *)R_alloc(ngram_max, sizeof(*buffer));
	ngram_set = (void *)R_alloc(ngram_max + 1, sizeof(*ngram_set));
	memset(ngram_set, 0, (ngram_max + 1) * sizeof(*ngram_set));
	if (sngrams != R_NilValue) {
		for (i = 0; i < XLENGTH(sngrams); i++) {
			if (ngrams[i] == NA_INTEGER) {
				continue;
			}
			ngram_set[ngrams[i]] = 1;
		}
	} else {
		for (i = 0; i < ngram_max; i++) {
			ngram_set[i + 1] = 1;
		}
	}

	weights = as_weights(sweights, n);
	if (sgroup != R_NilValue) {
		srow_names = getAttrib(sgroup, R_LevelsSymbol);
		ngroup = XLENGTH(srow_names);
		group = INTEGER(sgroup);
	} else {
		srow_names = names_text(sx);
		ngroup = n;
		group = NULL;
	}

	ngram = NULL;
	if (ngroup > 0) {
		if (!(ngram = malloc(ngroup * sizeof(*ngram)))) {
			CLEANUP();
			Rf_error("failed allocating %d objects"
				 " of size %d bytes", ngroup, sizeof(*ngram));
		}
	}

	for (has_ngram = 0; has_ngram < ngroup; has_ngram++) {
		if ((err = corpus_ngram_init(&ngram[has_ngram], ngram_max))) {
			goto out;
		}
	}

	for (i = 0; i < n; i++) {
		if (!group) {
			g = i;
		} else if (group[i] == NA_INTEGER) {
			continue;
		} else {
			assert(0 < group[i] && group[i] <= ngroup);
			g = (R_xlen_t)(group[i] - 1);
		}

		w = weights ? weights[i] : 1;

		if ((err = corpus_filter_start(filter, &text[i],
					       CORPUS_FILTER_SCAN_TOKENS))) {
			goto out;
		}

		if ((err = corpus_ngram_break(&ngram[g]))) {
			goto out;
		}

		while (corpus_filter_advance(filter)) {
			type_id = filter->type_id;
			if (type_id == CORPUS_FILTER_IGNORED) {
				continue;
			} else if (type_id < 0) {
				if ((err = corpus_ngram_break(&ngram[g]))) {
					goto out;
				}
				continue;
			}

			if ((err = corpus_ngram_add(&ngram[g], type_id, w))) {
				goto out;
			}
		}

		if ((err = filter->error)) {
			goto out;
		}
	}

	if (!select) {
		if ((err = corpus_termset_init(&termset))) {
			goto out;
		}
		has_termset = 1;
	}
	nz = 0;

	for (g = 0; g < ngroup; g++) {
		corpus_ngram_iter_make(&it, &ngram[g], buffer);
		while (corpus_ngram_iter_advance(&it)) {
			if (!ngram_set[it.length]) {
				continue;
			}

			if (select) {
			       if (!corpus_termset_has(&select->set,
						       it.type_ids,
						       it.length, NULL)) {
				       continue;
			       }
			} else {
				if ((err = corpus_termset_add(&termset,
							      it.type_ids,
							      it.length,
							      NULL))) {
				       goto out;
				}
			}

			if (nz == R_XLEN_T_MAX) {
				CLEANUP();
				Rf_error("overflow error;"
					 " number of matrix elements"
					 " exceeds maximum (%"PRIu64")",
					 (uint64_t)R_XLEN_T_MAX);
			}
			nz++;
		}
	}

	PROTECT(si = allocVector(REALSXP, nz)); nprot++;
	PROTECT(sj = allocVector(INTSXP, nz)); nprot++;
	PROTECT(scount = allocVector(REALSXP, nz)); nprot++;

	off = 0;
	terms = select ? &select->set : &termset;
	for (g = 0; g < ngroup; g++) {
		corpus_ngram_iter_make(&it, &ngram[g], buffer);
		while (corpus_ngram_iter_advance(&it)) {
			if (!ngram_set[it.length]) {
				continue;
			}

			if (!corpus_termset_has(terms, it.type_ids,
						it.length, &term_id)) {
				continue;
			}

			REAL(si)[off] = (double)g;
			INTEGER(sj)[off] = term_id;
			REAL(scount)[off] = it.weight;
			off++;
		}
	}

	PROTECT(scol_names = allocVector(STRSXP, terms->nitem));
	nprot++;

	if ((err = corpus_render_init(&render, CORPUS_ESCAPE_NONE))) {
		goto out;
	}
	has_render = 1;

	for (i = 0; i < terms->nitem; i++) {
		type_ids = terms->items[i].type_ids;
		m = terms->items[i].length;
		for (j = 0; j < m; j++) {
			type = corpus_filter_type(filter, type_ids[j]);
			if (j > 0) {
				corpus_render_char(&render, ' ');
			}
			corpus_render_text(&render, type);
		}
		if ((err = render.error)) {
			goto out;
		}
		sterm = mkCharLenCE(render.string, render.length, CE_UTF8);
		corpus_render_clear(&render);

		SET_STRING_ELT(scol_names, i, sterm);
	}

	PROTECT(ans = allocVector(VECSXP, 5)); nprot++;
	SET_VECTOR_ELT(ans, 0, si);
	SET_VECTOR_ELT(ans, 1, sj);
	SET_VECTOR_ELT(ans, 2, scount);
	SET_VECTOR_ELT(ans, 3, srow_names);
	SET_VECTOR_ELT(ans, 4, scol_names);

	PROTECT(snames = allocVector(STRSXP, 5)); nprot++;
	SET_STRING_ELT(snames, 0, mkChar("i"));
	SET_STRING_ELT(snames, 1, mkChar("j"));
	SET_STRING_ELT(snames, 2, mkChar("count"));
	SET_STRING_ELT(snames, 3, mkChar("row_names"));
	SET_STRING_ELT(snames, 4, mkChar("col_names"));
	setAttrib(ans, R_NamesSymbol, snames);

	err = 0;
out:
	CLEANUP();
	if (err) {
		Rf_error("failed computing term counts");
	}
	UNPROTECT(nprot);
	return ans;
}
