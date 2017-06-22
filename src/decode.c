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

#include <stdlib.h>
#include "corpus/src/error.h"
#include "corpus/src/text.h"
#include "corpus/src/unicode.h"
#include "corpus/src/data.h"
#include "rcorpus.h"

static int logical_data(const struct corpus_data *d);
static int integer_data(const struct corpus_data *d, int *overflowptr);
static double real_data(const struct corpus_data *d, int *overflowptr);
static SEXP charsxp_data(const struct corpus_data *d, struct mkchar *mk);
static SEXP decode_array(struct decode *d, const struct corpus_data *val,
			 const struct corpus_schema *s);
static SEXP decode_record(struct decode *d, const struct corpus_data *val,
			  const struct corpus_schema *s);


void decode_init(struct decode *d)
{
	mkchar_init(&d->mkchar);
	decode_clear(d);
}


void decode_clear(struct decode *d)
{
	d->overflow = 0;
}


int decode_logical(struct decode *d, const struct corpus_data *val)
{
	(void)d;
	return logical_data(val);
}


int decode_integer(struct decode *d, const struct corpus_data *val)
{
	return integer_data(val, &d->overflow);
}


double decode_real(struct decode *d, const struct corpus_data *val)
{
	return real_data(val, &d->overflow);
}


SEXP decode_charsxp(struct decode *d, const struct corpus_data *val)
{
	return charsxp_data(val, &d->mkchar);
}


SEXP decode_sexp(struct decode *d, const struct corpus_data *val,
		 const struct corpus_schema *s)
{
	SEXP ans;
	int kind, i;
	int overflow;

	if (val->type_id < 0) {
		error("invalid data object");
	}

	kind = s->types[val->type_id].kind;
	overflow = 0;

	switch (kind) {
	case CORPUS_DATATYPE_NULL:
		ans = R_NilValue;
		break;

	case CORPUS_DATATYPE_BOOLEAN:
		ans = ScalarLogical(logical_data(val));
		break;

	case CORPUS_DATATYPE_INTEGER:
		i = integer_data(val, &overflow);
		if (!overflow) {
			ans = ScalarInteger(i);
			break;
		}
		// else fall through to CORPUS_DATATYPE_REAL

	case CORPUS_DATATYPE_REAL:
		ans = ScalarReal(real_data(val, &overflow));
		break;

	case CORPUS_DATATYPE_TEXT:
		ans = ScalarString(charsxp_data(val, &d->mkchar));
		break;

	case CORPUS_DATATYPE_ARRAY:
		ans = decode_array(d, val, s);
		break;

	case CORPUS_DATATYPE_RECORD:
		ans = decode_record(d, val, s);
		break;

	default:
		ans = R_NilValue; // impossible, but silence warning anyway
		break;
	}

	if (overflow) {
		d->overflow = overflow;
	}

	return ans;
}


int logical_data(const struct corpus_data *d)
{
	int ans, err, b;

	err = corpus_data_bool(d, &b);

	if (err == CORPUS_ERROR_INVAL) {
		ans = NA_LOGICAL;
	} else {
		ans = b ? TRUE : FALSE;
	}

	return ans;
}


int integer_data(const struct corpus_data *d, int *overflowptr)
{
	int ans, err, i;
	int overflow;

	overflow = 0;

	err = corpus_data_int(d, &i);
	if (err == CORPUS_ERROR_INVAL) {
		ans = NA_INTEGER;
	} else if (err == CORPUS_ERROR_OVERFLOW || i == NA_INTEGER) {
		ans = NA_INTEGER;
		overflow = 1;
	} else {
		ans = i;
	}

	if (overflowptr) {
		*overflowptr = overflow;
	}

	return ans;
}


double real_data(const struct corpus_data *d, int *overflowptr)
{
	double ans, r;
	int err;
	int overflow;

	overflow = 0;
	err = corpus_data_double(d, &r);
	if (err == CORPUS_ERROR_INVAL) {
		ans = NA_REAL;
	} else {
		if (err == CORPUS_ERROR_OVERFLOW) {
			overflow = 1;
		}
		ans = r;
	}

	if (overflowptr) {
		*overflowptr = overflow;
	}

	return ans;
}


SEXP charsxp_data(const struct corpus_data *d, struct mkchar *mk)
{
	struct corpus_text text;
	int err;

	err = corpus_data_text(d, &text);
	if (err == CORPUS_ERROR_INVAL) {
		return NA_STRING;
	}

	return mkchar_get(mk, &text);
}


SEXP decode_array(struct decode *d, const struct corpus_data *val,
		  const struct corpus_schema *s)
{
	SEXP ans;
	struct corpus_data_items it;
	int err, i, n, overflow;
	int arr_id, type_id;

	if ((err = corpus_data_nitem(val, s, &n))) {
		ans = R_NilValue;
		goto out;
	}
	corpus_data_items(val, s, &it); // won't fail if data_nitem succeeds
	i = 0;

	arr_id = val->type_id;
	type_id = s->types[arr_id].meta.array.type_id;
	switch (type_id) {
	case CORPUS_DATATYPE_BOOLEAN:
		PROTECT(ans = allocVector(LGLSXP, n));
		while (corpus_data_items_advance(&it)) {
			LOGICAL(ans)[i] = decode_logical(d, &it.current);
			i++;
		}
		break;

	case CORPUS_DATATYPE_INTEGER:
		PROTECT(ans = allocVector(INTSXP, n));
		overflow = 0;
		while (corpus_data_items_advance(&it)) {
			INTEGER(ans)[i] = integer_data(&it.current, &overflow);
			i++;
		}
		if (!overflow) {
			break;
		} else {
			UNPROTECT(1);
			corpus_data_items_reset(&it);
			// fall through, decode as CORPUS_DATATYPE_REAL
		}

	case CORPUS_DATATYPE_REAL:
		PROTECT(ans = allocVector(REALSXP, n));
		while (corpus_data_items_advance(&it)) {
			REAL(ans)[i] = decode_real(d, &it.current);
			i++;
		}
		break;

	case CORPUS_DATATYPE_TEXT:
		PROTECT(ans = allocVector(STRSXP, n));
		while (corpus_data_items_advance(&it)) {
			SET_STRING_ELT(ans, i, decode_charsxp(d, &it.current));
			i++;
		}
		break;

	default:
		if (n == 0) {
			ans = R_NilValue;
			goto out;
		}

		PROTECT(ans = allocVector(VECSXP, n));
		while (corpus_data_items_advance(&it)) {
			SET_VECTOR_ELT(ans, i, decode_sexp(d, &it.current, s));
			i++;
		}
		break;
	}


	UNPROTECT(1);

out:
	return ans;
}


SEXP decode_record(struct decode *d, const struct corpus_data *val,
		   const struct corpus_schema *s)
{
	SEXP ans, names;
	const struct corpus_text *name;
	struct corpus_data_fields it;
	int err, i, n;

	if ((err = corpus_data_nfield(val, s, &n))) {
		ans = R_NilValue;
		goto out;
	}
	corpus_data_fields(val, s, &it); // won't fail if data_nfield succeeds

	PROTECT(ans = allocVector(VECSXP, n));
	PROTECT(names = allocVector(STRSXP, n));

	if (n > 0) {
		setAttrib(ans, R_NamesSymbol, names);
	}

	i = 0;
	while (corpus_data_fields_advance(&it)) {
		SET_VECTOR_ELT(ans, i, decode_sexp(d, &it.current, s));

		name = &s->names.types[it.name_id].text;
		SET_STRING_ELT(names, i, mkchar_get(&d->mkchar, name));
		i++;
	}
	UNPROTECT(2);

out:
	return ans;
}
