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
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <Rdefines.h>
#include "corpus/src/error.h"
#include "corpus/src/filebuf.h"
#include "corpus/src/render.h"
#include "corpus/src/table.h"
#include "corpus/src/text.h"
#include "corpus/src/textset.h"
#include "corpus/src/typemap.h"
#include "corpus/src/symtab.h"
#include "corpus/src/unicode.h"
#include "corpus/src/data.h"
#include "corpus/src/datatype.h"
#include "rcorpus.h"

#define JSON_TAG install("corpus::json")


static SEXP subrows_json(SEXP sdata, SEXP si);
static SEXP subfield_json(SEXP sdata, SEXP sname);


static void free_json(SEXP sjson)
{
        struct json *d = R_ExternalPtrAddr(sjson);
	if (d) {
		free(d->rows);
		free(d);
	}
}


SEXP alloc_json(SEXP sbuffer, SEXP sfield, SEXP srows)
{
	SEXP ans, sclass, shandle, snames;
	struct json *obj;
	int err;

	PROTECT(shandle = R_MakeExternalPtr(NULL, JSON_TAG, R_NilValue));
	R_RegisterCFinalizerEx(shandle, free_json, TRUE);

	if (!(obj = malloc(sizeof(*obj)))) {
		error("failed allocating memory (%u bytes)",
			(unsigned)sizeof(*obj));
	}
	if ((err = corpus_schema_init(&obj->schema))) {
		free(obj);
		error("failed allocating memory");
	}

	obj->rows = NULL;
	obj->nrow = 0;
	obj->type_id = CORPUS_DATATYPE_NULL;
	obj->kind = CORPUS_DATATYPE_NULL;

	R_SetExternalPtrAddr(shandle, obj);

	PROTECT(ans = allocVector(VECSXP, 4));
	SET_VECTOR_ELT(ans, 0, shandle);
	SET_VECTOR_ELT(ans, 1, sbuffer);
	SET_VECTOR_ELT(ans, 2, sfield);
	SET_VECTOR_ELT(ans, 3, srows);

	PROTECT(snames = allocVector(STRSXP, 4));
	SET_STRING_ELT(snames, 0, mkChar("handle"));
	SET_STRING_ELT(snames, 1, mkChar("buffer"));
	SET_STRING_ELT(snames, 2, mkChar("field"));
	SET_STRING_ELT(snames, 3, mkChar("rows"));
	setAttrib(ans, R_NamesSymbol, snames);

	PROTECT(sclass = allocVector(STRSXP, 1));
	SET_STRING_ELT(sclass, 0, mkChar("corpus_json"));
	setAttrib(ans, R_ClassSymbol, sclass);

	UNPROTECT(4);
	return ans;
}


static void grow_datarows(struct corpus_data **rowsptr, R_xlen_t *nrow_maxptr)
{
	void *base1, *base = *rowsptr;
	size_t size1, size = (size_t)*nrow_maxptr;
	size_t width = sizeof(**rowsptr);

	if (size == 0) {
		size1 = 1;
	} else {
		size1 = 1.618 * size + 1; // (golden ratio)
	}

	if (size1 < size) { // overflow
		size1 = SIZE_MAX;
	}

	if (size1 > SIZE_MAX / width) {
		free(base);
		error("number of rows (%"PRIu64")"
			" exceeds maximum (%"PRIu64")",
			(uint64_t)size1, (uint64_t)SIZE_MAX / width);
	}
	if (size1 > R_XLEN_T_MAX) {
		free(base);
		error("number of rows (%"PRIu64") exceeds maximum (%"PRIu64")",
			(uint64_t)size1, (uint64_t)R_XLEN_T_MAX);
	}
	if (size1 > (((uint64_t)1) << DBL_MANT_DIG)) {
		error("number of rows (%"PRIu64") exceeds maximum (%"PRIu64")",
			(uint64_t)size1, ((uint64_t)1) << DBL_MANT_DIG);
	}

	base1 = realloc(base, size1 * width);
	if (size1 > 0 && base1 == NULL) {
		free(base);
		error("failed allocating %"PRIu64" bytes",
			(uint64_t)size1 * width);
	}

	*rowsptr = base1;
	*nrow_maxptr = size1;
}


static void json_load(SEXP sdata)
{
	SEXP shandle, sparent_handle, sbuffer, sfield, sfield_path,
	     srows, sparent, sparent2;
	struct json *obj;
	struct corpus_data *rows;
	struct corpus_filebuf *buf;
	struct corpus_filebuf_iter it;
	const uint8_t *ptr, *begin, *line_end, *end;
	uint_fast8_t ch;
	size_t size;
	R_xlen_t nrow, nrow_max, j, m;
	int err, type_id;

	shandle = getListElement(sdata, "handle");
	obj = R_ExternalPtrAddr(shandle);
	if (obj && obj->rows) {
		return;
	}

	sbuffer = getListElement(sdata, "buffer");
	PROTECT(sparent = alloc_json(sbuffer, R_NilValue, R_NilValue));
	sparent_handle = getListElement(sparent, "handle");
	obj = R_ExternalPtrAddr(sparent_handle);

	type_id = CORPUS_DATATYPE_NULL;
	nrow = 0;
	nrow_max = 0;
	rows = NULL;

	if (is_filebuf(sbuffer)) {
		buf = as_filebuf(sbuffer);

		corpus_filebuf_iter_make(&it, buf);
		while (corpus_filebuf_iter_advance(&it)) {
			if (nrow == nrow_max) {
				grow_datarows(&rows, &nrow_max);
			}

			ptr = it.current.ptr;
			size = it.current.size;

			if ((err = corpus_data_assign(&rows[nrow],
						      &obj->schema,
						      ptr, size))) {
				free(rows);
				error("error parsing row %"PRIu64
				      " of JSON data", (uint64_t)(nrow + 1));
			}

			if ((err = corpus_schema_union(&obj->schema, type_id,
						       rows[nrow].type_id,
						       &type_id))) {
				free(rows);
				error("memory allocation failure"
				      " after parsing row %"PRIu64
				      " of JSON data", (uint64_t)(nrow + 1));
			}
			nrow++;
		}
	} else {
		// parse data from buffer
		begin = (const uint8_t *)RAW(sbuffer);
		end = begin + XLENGTH(sbuffer);
		ptr = begin;

		while (ptr != end) {
			if (nrow == nrow_max) {
				grow_datarows(&rows, &nrow_max);
			}

			line_end = ptr;
			do {
				ch = *line_end++;
			} while (ch != '\n' && line_end != end);

			size = (size_t)(line_end - ptr);

			if ((err = corpus_data_assign(&rows[nrow],
						      &obj->schema,
						      ptr, size))) {
				free(rows);
				error("error parsing row %"PRIu64
				      " of JSON data", (uint64_t)(nrow + 1));
			}

			if ((err = corpus_schema_union(&obj->schema, type_id,
						       rows[nrow].type_id,
						       &type_id))) {
				free(rows);
				error("memory allocation failure"
				      " after parsing row %"PRIu64
				      " of JSON data", (uint64_t)(nrow + 1));
			}
			nrow++;
			ptr = line_end;
		}
	}

	// free excess memory
	rows = realloc(rows, nrow * sizeof(*rows));

	// ensure rows is non-NULL even if nrow == 0
	if (rows == NULL) {
		rows = malloc(sizeof(*rows));
		if (!rows) {
			error("failed allocating memory (%u bytes)",
				sizeof(*rows));
		}
	}

	// set the fields
	obj->rows = rows;
	obj->nrow = nrow;
	obj->type_id = type_id;
	if (type_id < 0) {
		obj->kind = CORPUS_DATATYPE_ANY;
	} else {
		obj->kind = obj->schema.types[type_id].kind;
	}

	// first extract the rows from the parent...
	srows = getListElement(sdata, "rows");
	if (srows != R_NilValue) {
		PROTECT(sparent2 = subrows_json(sparent, srows));
		free_json(sparent_handle);
		R_SetExternalPtrAddr(sparent_handle, NULL);
		UNPROTECT(2);
		PROTECT(sparent = sparent2);
		sparent_handle = getListElement(sparent, "handle");
	}

	// ...then extract the field
	sfield_path = getListElement(sdata, "field");
	if (sfield_path != R_NilValue) {
		m = XLENGTH(sfield_path);
		for (j = 0; j < m; j++) {
			sfield = STRING_ELT(sfield_path, j);
			PROTECT(sparent2 = subfield_json(sparent, sfield));
			free_json(sparent_handle);
			R_SetExternalPtrAddr(sparent_handle, NULL);
			UNPROTECT(2);
			PROTECT(sparent = sparent2);
			sparent_handle = getListElement(sparent, "handle");
		}
	}

	// steal the handle from the parent
	free_json(shandle);
	R_SetExternalPtrAddr(shandle, R_ExternalPtrAddr(sparent_handle));
	R_SetExternalPtrAddr(sparent_handle, NULL);
	UNPROTECT(1);
}


int is_json(SEXP sdata)
{
	SEXP handle, buffer;

	if (!isVectorList(sdata)) {
		return 0;
	}

	handle = getListElement(sdata, "handle");
	if (handle == R_NilValue) {
		return 0;
	}

	buffer = getListElement(sdata, "buffer");
	if (!(TYPEOF(buffer) == RAWSXP || is_filebuf(buffer))) {
		return 0;
	}

	return ((TYPEOF(handle) == EXTPTRSXP)
		&& (R_ExternalPtrTag(handle) == JSON_TAG));
}


struct json *as_json(SEXP sdata)
{
	SEXP shandle;
	struct json *obj;

	if (!is_json(sdata)) {
		error("invalid 'json' object");
	}

	json_load(sdata);

	shandle = getListElement(sdata, "handle");
	obj = R_ExternalPtrAddr(shandle);

	return obj;
}


SEXP dim_json(SEXP sdata)
{
	SEXP dims;
	const struct json *d = as_json(sdata);
	const struct corpus_datatype *t;
	const struct corpus_datatype_record *r;

	if (d->kind != CORPUS_DATATYPE_RECORD) {
		return R_NilValue;
	}

	t = &d->schema.types[d->type_id];
	r = &t->meta.record;

	if (d->nrow > INT_MAX) {
		PROTECT(dims = allocVector(REALSXP, 2));
		REAL(dims)[0] = (double)d->nrow;
		REAL(dims)[1] = (double)r->nfield;
	} else {
		PROTECT(dims = allocVector(INTSXP, 2));
		INTEGER(dims)[0] = d->nrow;
		INTEGER(dims)[1] = (int)r->nfield;
	}
	UNPROTECT(1);

	return dims;
}


SEXP length_json(SEXP sdata)
{
	const struct json *d = as_json(sdata);
	const struct corpus_datatype *t;
	const struct corpus_datatype_record *r;

	if (d->kind == CORPUS_DATATYPE_RECORD) {
		t = &d->schema.types[d->type_id];
		r = &t->meta.record;
		return ScalarInteger(r->nfield);
	}

	if (d->nrow > INT_MAX) {
		return ScalarReal((double)d->nrow);
	} else {
		return ScalarInteger((int)d->nrow);
	}
}


SEXP names_json(SEXP sdata)
{
	SEXP names, str;
	const struct json *d = as_json(sdata);
	const struct corpus_datatype *t;
	const struct corpus_datatype_record *r;
	const struct corpus_text *name;
	int i;

	if (d->kind != CORPUS_DATATYPE_RECORD) {
		return R_NilValue;
	}

	t = &d->schema.types[d->type_id];
	r = &t->meta.record;

	PROTECT(names = allocVector(STRSXP, r->nfield));
	for (i = 0; i < r->nfield; i++) {
		name = &d->schema.names.types[r->name_ids[i]].text;
		str = mkCharLenCE((char *)name->ptr, CORPUS_TEXT_SIZE(name),
				  CE_UTF8);
		SET_STRING_ELT(names, i, str);
	}

	UNPROTECT(1);
	return names;
}


SEXP json_datatype(SEXP sdata)
{
	SEXP str, ans;
	const struct json *d = as_json(sdata);
	struct corpus_render r;

	if (corpus_render_init(&r, CORPUS_ESCAPE_NONE) != 0) {
		error("memory allocation failure");
	}
	corpus_render_set_tab(&r, "");
	corpus_render_set_newline(&r, " ");

	corpus_render_datatype(&r, &d->schema, d->type_id);
	if (r.error) {
		corpus_render_destroy(&r);
		error("memory allocation failure");
	}

	PROTECT(ans = allocVector(STRSXP, 1));
	str = mkCharLenCE(r.string, r.length, CE_UTF8);
	SET_STRING_ELT(ans, 0, str);

	corpus_render_destroy(&r);
	UNPROTECT(1);
	return ans;
}


SEXP json_datatypes(SEXP sdata)
{
	SEXP types, str, names;
	const struct json *d = as_json(sdata);
	const struct corpus_datatype *t;
	const struct corpus_datatype_record *rec;
	struct corpus_render r;
	int i;

	if (d->kind != CORPUS_DATATYPE_RECORD) {
		return R_NilValue;
	}

	PROTECT(names = names_json(sdata));

	t = &d->schema.types[d->type_id];
	rec = &t->meta.record;

	if (corpus_render_init(&r, CORPUS_ESCAPE_NONE) != 0) {
		error("memory allocation failure");
	}
	corpus_render_set_tab(&r, "");
	corpus_render_set_newline(&r, " ");

	PROTECT(types = allocVector(STRSXP, rec->nfield));
	for (i = 0; i < rec->nfield; i++) {
		corpus_render_datatype(&r, &d->schema, rec->type_ids[i]);
		if (r.error) {
			corpus_render_destroy(&r);
			error("memory allocation failure");
		}
		str = mkCharLenCE(r.string, r.length, CE_UTF8);
		SET_STRING_ELT(types, i, str);
		corpus_render_clear(&r);
	}
	setAttrib(types, R_NamesSymbol, names);

	corpus_render_destroy(&r);
	UNPROTECT(2);
	return types;
}


SEXP print_json(SEXP sdata)
{
	const struct json *d = as_json(sdata);
	struct corpus_render r;

	if (corpus_render_init(&r, CORPUS_ESCAPE_CONTROL) != 0) {
		error("memory allocation failure");
	}

	corpus_render_datatype(&r, &d->schema, d->type_id);
	if (r.error) {
		corpus_render_destroy(&r);
		error("memory allocation failure");
	}

	if (d->kind == CORPUS_DATATYPE_RECORD) {
		Rprintf("JSON data set with %"PRIu64" rows"
			" of the following type:\n%s\n",
			(uint64_t)d->nrow, r.string);
	} else {
		Rprintf("JSON data set with %"PRIu64" rows"
			" of type %s\n", (uint64_t)d->nrow, r.string);
	}

	corpus_render_destroy(&r);
	return sdata;
}


SEXP subscript_json(SEXP sdata, SEXP si)
{
	SEXP ans, sname;
	const struct json *d = as_json(sdata);
	const struct corpus_schema *s = &d->schema;
	const struct corpus_datatype *t;
	const struct corpus_datatype_record *r;
	const struct corpus_text *name;
	double i;
	int name_id;

	if (!(isReal(si) && LENGTH(si) == 1)) {
		error("invalid 'i' argument");
	}
	i = REAL(si)[0];

	if (d->kind != CORPUS_DATATYPE_RECORD) {
		ans = subrows_json(sdata, si);
	} else {
		t = &d->schema.types[d->type_id];
		r = &t->meta.record;

		if (!(1 <= i && i <= r->nfield)) {
			error("invalid subscript: %g", i);
		}
		name_id = r->name_ids[(int)(i - 1)];
		name = &s->names.types[name_id].text;

		PROTECT(sname = mkCharLenCE((const char *)name->ptr,
					    (int)CORPUS_TEXT_SIZE(name),
					    CE_UTF8));
		PROTECT(ans = subfield_json(sdata, sname));
		UNPROTECT(2);
	}

	return ans;
}


SEXP subrows_json(SEXP sdata, SEXP si)
{
	SEXP ans, shandle, sbuffer, sfield, srows, srows2;
	const struct json *obj = as_json(sdata);
	struct json *obj2;
	struct corpus_data *rows;
	const struct corpus_data *src;
	const double *index;
	double *irows;
	R_xlen_t i, n, ind;
	int type_id;
	int err;

	if (si == R_NilValue) {
		return sdata;
	}

	index = REAL(si);
	n = XLENGTH(si);

	sbuffer = getListElement(sdata, "buffer");
	sfield = getListElement(sdata, "field");
	srows = getListElement(sdata, "rows");

	PROTECT(srows2 = allocVector(REALSXP, n));
	irows = REAL(srows2);

	PROTECT(ans = alloc_json(sbuffer, sfield, srows2));
	shandle = getListElement(ans, "handle");
	obj2 = R_ExternalPtrAddr(shandle);

	rows = malloc(n * sizeof(*rows));
	obj2->rows = rows;

	if (n > 0 && !rows) {
		error("failed allocating %"PRIu64" bytes",
		      (uint64_t)n * sizeof(*rows));
	}

	type_id = CORPUS_DATATYPE_NULL;

	for (i = 0; i < n; i++) {
		if (!(1 <= index[i] && index[i] <= (double)obj->nrow)) {
			free(rows);
			error("invalid index: %g", index[i]);
		}

		ind = (R_xlen_t)(index[i] - 1);
		if (srows == R_NilValue) {
			irows[i] = index[i];
		} else {
			irows[i] = REAL(srows)[ind];
		}
		src = &obj->rows[ind];

		if ((err = corpus_data_assign(&rows[i], &obj2->schema,
					      src->ptr, src->size))) {
			error("error parsing row %"PRIu64
			      " of JSON file", (uint64_t)(irows[i] + 1));
		}

		if ((err = corpus_schema_union(&obj2->schema, type_id,
					       rows[i].type_id, &type_id))) {
			error("memory allocation failure"
			      " after parsing row %"PRIu64
			      " of JSON file", (uint64_t)(irows[i] + 1));
		}

	}

	// set the fields
	obj2->nrow = n;
	obj2->type_id = type_id;

	if (type_id < 0) {
		obj2->kind = CORPUS_DATATYPE_ANY;
	} else {
		obj2->kind = obj2->schema.types[type_id].kind;
	}

	UNPROTECT(2);
	return ans;
}


SEXP subfield_json(SEXP sdata, SEXP sname)
{
	SEXP ans, sbuffer, sfield, sfield2, shandle, srows;
	const struct json *obj = as_json(sdata);
	struct corpus_text name;
	struct corpus_data *rows;
	struct corpus_data field;
	const char *name_ptr;
	size_t name_len;
	struct json *obj2;
	R_xlen_t i, n;
	int err, j, m, name_id, type_id;

	if (sname == R_NilValue) {
		return sdata;
	} else if (TYPEOF(sname) != CHARSXP) {
                error("invalid 'name' argument");
        }
	name_ptr = translateCharUTF8(sname);
	name_len = strlen(name_ptr);
	PROTECT(sname = mkCharLenCE(name_ptr, name_len, CE_UTF8));
	if ((err = corpus_text_assign(&name, (uint8_t *)name_ptr, name_len,
				      CORPUS_TEXT_NOESCAPE))) {
		error("invalid UTF-8 in 'name' argument");
	}
	if (!corpus_symtab_has_type(&obj->schema.names, &name, &name_id)) {
		UNPROTECT(1);
		return R_NilValue;
	}

	sbuffer = getListElement(sdata, "buffer");
	sfield = getListElement(sdata, "field");
	srows = getListElement(sdata, "rows");

	if (sfield == R_NilValue) {
		m = 0;
	} else {
		m = LENGTH(sfield);
	}

	PROTECT(sfield2 = allocVector(STRSXP, m + 1));
	for (j = 0; j < m; j++) {
		SET_STRING_ELT(sfield2, j, STRING_ELT(sfield, j));
	}
	SET_STRING_ELT(sfield2, m, sname);

	PROTECT(ans = alloc_json(sbuffer, sfield2, srows));
	shandle = getListElement(ans, "handle");
	obj2 = R_ExternalPtrAddr(shandle);

	n = obj->nrow;
	rows = malloc(n * sizeof(*rows));

	if (n > 0 && !rows) {
		error("failed allocating %"PRIu64" bytes",
		      (uint64_t)n * sizeof(*rows));
	}
	obj2->rows = rows;

	type_id = CORPUS_DATATYPE_NULL;
	for (i = 0; i < n; i++) {
		corpus_data_field(&obj->rows[i], &obj->schema, name_id,
				  &field);
		corpus_data_assign(&rows[i], &obj2->schema, field.ptr,
				   field.size);
		if (corpus_schema_union(&obj2->schema, type_id,
					rows[i].type_id, &type_id) != 0) {
			error("memory allocation failure");
		}
	}

	obj2->nrow = n;
	obj2->type_id = type_id;

	if (type_id < 0) {
		obj2->kind = CORPUS_DATATYPE_ANY;
	} else {
		obj2->kind = obj2->schema.types[type_id].kind;
	}

	UNPROTECT(3);
	return ans;
}


SEXP subset_json(SEXP sdata, SEXP si, SEXP sj)
{
	SEXP ans;
	const struct json *d = as_json(sdata);

	if (si == R_NilValue) {
		if (sj == R_NilValue) {
			return sdata;
		} else {
			if (d->kind != CORPUS_DATATYPE_RECORD) {
				error("incorrect number of dimensions");
			}
			return subscript_json(sdata, sj);
		}
	} else if (sj == R_NilValue) {
		return subrows_json(sdata, si);
	} else {
		if (d->kind != CORPUS_DATATYPE_RECORD) {
			error("incorrect number of dimensions");
		}

		PROTECT(sdata = subrows_json(sdata, si));
		ans = subscript_json(sdata, sj);
		UNPROTECT(1);
		return ans;
	}
}


SEXP as_double_json(SEXP sdata)
{
	SEXP ans;
	const struct json *d = as_json(sdata);
	double *val;
	R_xlen_t i, n = d->nrow;
	int err, overflow;

	PROTECT(ans = allocVector(REALSXP, n));
	val = REAL(ans);
	overflow = 0;

	for (i = 0; i < n; i++) {
		err = corpus_data_double(&d->rows[i], &val[i]);
		if (err == CORPUS_ERROR_INVAL) {
			val[i] = NA_REAL;
		} else if (err == CORPUS_ERROR_OVERFLOW) {
			overflow = 1;
		}
	}

	if (overflow) {
		warning("NAs introduced by coercion to double range");
	}

	UNPROTECT(1);
	return ans;
}


static SEXP as_integer_json_check(SEXP sdata, int *overflowptr)
{
	SEXP ans;
	const struct json *d = as_json(sdata);
	int *val;
	R_xlen_t i, n = d->nrow;
	int err, overflow;

	PROTECT(ans = allocVector(INTSXP, n));
	val = INTEGER(ans);
	overflow = 0;

	for (i = 0; i < n; i++) {
		err = corpus_data_int(&d->rows[i], &val[i]);
		if (err == CORPUS_ERROR_INVAL) {
			val[i] = NA_INTEGER;
		} else {
			if (err == CORPUS_ERROR_OVERFLOW
					|| val[i] == NA_INTEGER) {
				overflow = 1;
				val[i] = NA_INTEGER;
			}
		}
	}

	if (overflowptr) {
		*overflowptr = overflow;
	}

	UNPROTECT(1);
	return ans;
}


SEXP as_integer_json(SEXP sdata)
{
	SEXP ans;
	int overflow;

	PROTECT(ans = as_integer_json_check(sdata, &overflow));
	if (overflow) {
		warning("NAs introduced by coercion to integer range");
	}

	UNPROTECT(1);
	return ans;
}


SEXP as_logical_json(SEXP sdata)
{
	SEXP ans;
	const struct json *d = as_json(sdata);
	R_xlen_t i, n = d->nrow;
	int *val;
	int b, err;

	PROTECT(ans = allocVector(LGLSXP, n));
	val = LOGICAL(ans);

	for (i = 0; i < n; i++) {
		err = corpus_data_bool(&d->rows[i], &b);
		if (err == CORPUS_ERROR_INVAL) {
			val[i] = NA_LOGICAL;
		} else {
			val[i] = b ? TRUE : FALSE;
		}
	}

	UNPROTECT(1);
	return ans;
}


SEXP as_character_json(SEXP sdata)
{
	SEXP ans;
	const struct json *d = as_json(sdata);
	struct mkchar mkchar;
	struct corpus_text text;
	R_xlen_t i, n = d->nrow;
	int err;

	PROTECT(ans = allocVector(STRSXP, n));

	mkchar_init(&mkchar);

	for (i = 0; i < n; i++) {
		err = corpus_data_text(&d->rows[i], &text);
		if (err == CORPUS_ERROR_INVAL) {
			SET_STRING_ELT(ans, i, NA_STRING);
		} else {
			SET_STRING_ELT(ans, i, mkchar_get(&mkchar, &text));
		}
	}

	UNPROTECT(1);
	return ans;
}


SEXP as_factor_json(SEXP sdata)
{
	SEXP ans, lev, levels;
	const struct json *d = as_json(sdata);
	struct corpus_text text;
	struct corpus_text_iter it;
	struct corpus_textset set;
	struct mkchar mkchar;
	R_xlen_t i, n = d->nrow;
	int err, id, utf8, nprot = 0;
	struct corpus_text buf;
	uint8_t *ptr;
	size_t nbuf;

	buf.ptr = NULL;
	buf.attr = 0;
	nbuf = 0;

	PROTECT(ans = allocVector(INTSXP, n)); nprot++;

	if ((err = corpus_textset_init(&set))) {
		goto error_init;
	}

	for (i = 0; i < n; i++) {
		err = corpus_data_text(&d->rows[i], &text);
		if (err == CORPUS_ERROR_INVAL) {
			INTEGER(ans)[i] = NA_INTEGER;
		} else {
			// decode escapes in the input
			if (CORPUS_TEXT_HAS_ESC(&text)) {
				if (CORPUS_TEXT_SIZE(&text) >= nbuf) {
					nbuf = CORPUS_TEXT_SIZE(&text);
					ptr = realloc(buf.ptr, nbuf);
					if (!ptr) {
						corpus_textset_destroy(&set);
						free(buf.ptr);
						error("failed allocating"
						      " %"PRIu64" bytes",
						      (uint64_t)nbuf);
					}
					buf.ptr = ptr;
				}

				utf8 = 0;
				ptr = buf.ptr;
				corpus_text_iter_make(&it, &text);

				while (corpus_text_iter_advance(&it)) {
					if (!CORPUS_IS_ASCII(it.current)) {
						utf8 = 1;
					}
					corpus_encode_utf8(it.current,
							   &ptr);
				}

				buf.attr = (size_t)(ptr - buf.ptr);
				if (utf8) {
					buf.attr |= CORPUS_TEXT_UTF8_BIT;
				}

				err = corpus_textset_add(&set, &buf, &id);
			} else {
				err = corpus_textset_add(&set, &text, &id);
			}

			if (err) {
				goto error_add;
			}
			if (id == INT_MAX || id + 1 == NA_INTEGER) {
				corpus_textset_destroy(&set);
				free(buf.ptr);
				error("number of factor levels (%d)"
				      " exceeds maximum", id);
			}
			INTEGER(ans)[i] = id + 1;
		}
	}

	PROTECT(levels = allocVector(STRSXP, set.nitem)); nprot++;

	mkchar_init(&mkchar);

	for (id = 0; id < set.nitem; id++) {
		lev = mkchar_get(&mkchar, &set.items[id]);
		SET_STRING_ELT(levels, id, lev);
	}

	setAttrib(ans, R_LevelsSymbol, levels);
	setAttrib(ans, R_ClassSymbol, mkString("factor"));
	err = 0;

error_add:
	corpus_textset_destroy(&set);
	free(buf.ptr);
error_init:
	if (err) {
		error("failed decoding JSON data to factor type");
	}
	UNPROTECT(nprot);
	return ans;
}


static int in_string_set(SEXP strs, SEXP item)
{
	R_xlen_t i, n;
	const char *s1, *s2;

	if (strs == R_NilValue || item == NA_STRING) {
		return 0;
	}

	n = XLENGTH(strs);
	if (n == 0) {
		return 0;
	}

	s2 = translateCharUTF8(item);

	for (i = 0; i < n; i++) {
		if (STRING_ELT(strs, i) == NA_STRING) {
			continue;
		}

		s1 = translateCharUTF8(STRING_ELT(strs, i));
		if (strcmp(s1, s2) == 0) {
			return 1;
		}
	}

	return 0;
}


static SEXP as_list_json_record(SEXP sdata, SEXP stext,
				    SEXP stringsAsFactors)
{
	SEXP ans, ans_j, names, sbuffer, sfield, sfield2, srows,
	     shandle, sname;
	const struct json *d = as_json(sdata);
	struct json *d_j;
	struct corpus_schema **schema;
	const struct corpus_datatype_record *r;
	struct corpus_data_fields it;
	R_xlen_t i, n = d->nrow, k, m;
	int err, j, nfield;
	int *type_id;
	struct corpus_data **rows;
	int *cols;

	if (d->kind != CORPUS_DATATYPE_RECORD) {
		return R_NilValue;
	}

	r = &d->schema.types[d->type_id].meta.record;
	nfield = r->nfield;

	sbuffer = getListElement(sdata, "buffer");
	sfield = getListElement(sdata, "field");
	srows = getListElement(sdata, "rows");
	PROTECT(names = names_json(sdata));

	PROTECT(ans = allocVector(VECSXP, r->nfield));
	setAttrib(ans, R_NamesSymbol, names);
	rows = (struct corpus_data **)R_alloc(nfield, sizeof(*rows));
	cols = (int *)R_alloc(d->schema.names.ntype, sizeof(*cols));
	schema = (struct corpus_schema **)R_alloc(nfield, sizeof(*schema));
	type_id = (int *)R_alloc(nfield, sizeof(type_id));

	for (j = 0; j < nfield; j++) {
		// use calloc so that all items are initialized to null
		rows[j] = calloc(n, sizeof(*rows[j]));
		if (!rows[j] && n) {
			error("failed allocating memory (%"PRIu64" bytes)",
			      (uint64_t)n * sizeof(*rows[j]));
		}
		cols[r->name_ids[j]] = j;

		sname = STRING_ELT(names, j);
		m = (sfield == R_NilValue) ? 0 : XLENGTH(sfield);

		PROTECT(sfield2 = allocVector(STRSXP, m + 1));
		for (k = 0; k < m; k++) {
			SET_STRING_ELT(sfield2, k, STRING_ELT(sfield, k));
		}
		SET_STRING_ELT(sfield2, m, sname);
		ans_j = alloc_json(sbuffer, sfield2, srows);
		SET_VECTOR_ELT(ans, j, ans_j);
		UNPROTECT(1); // sfield2 protected by ans_j, protected by ans

		shandle = getListElement(ans_j, "handle");
		d_j = R_ExternalPtrAddr(shandle);
		d_j->rows = rows[j];
		d_j->nrow = n;
		schema[j] = &d_j->schema;
		type_id[j] = CORPUS_DATATYPE_NULL;
	}

	for (i = 0; i < n; i++) {
		if ((err = corpus_data_fields(&d->rows[i], &d->schema, &it))) {
			continue;
		}

		while (corpus_data_fields_advance(&it)) {
			j = cols[it.name_id];
			if ((err = corpus_data_assign(&rows[j][i], schema[j],
						      it.current.ptr,
						      it.current.size))) {
				error("failed parsing field value");
			}

			if ((err = corpus_schema_union(schema[j],
						       rows[j][i].type_id,
						       type_id[j],
						       &type_id[j]))) {
				error("memory allocation failure");
			}
		}
	}

	for (j = 0; j < nfield; j++) {
		ans_j = VECTOR_ELT(ans, j);
		shandle = getListElement(ans_j, "handle");
		d_j = R_ExternalPtrAddr(shandle);
		d_j->type_id = type_id[j];
		if (type_id[j] < 0) {
			d_j->kind = CORPUS_DATATYPE_ANY;
		} else {
			d_j->kind = schema[j]->types[type_id[j]].kind;
		}

		if (d_j->kind == CORPUS_DATATYPE_TEXT
				&& in_string_set(stext,
						 STRING_ELT(names, j))) {
			ans_j = as_text_json(ans_j);
		} else {
			ans_j = simplify_json(ans_j, stext, stringsAsFactors);
		}
		SET_VECTOR_ELT(ans, j, ans_j);
	}

	UNPROTECT(2);
	return ans;
}


SEXP as_list_json(SEXP sdata, SEXP stext, SEXP stringsAsFactors)
{
	SEXP ans, val;
	const struct json *d = as_json(sdata);
	struct decode decode;
	struct corpus_data data;
	R_xlen_t i, n = d->nrow;

	if (d->kind == CORPUS_DATATYPE_RECORD) {
		return as_list_json_record(sdata, stext, stringsAsFactors);
	}

	PROTECT(ans = allocVector(VECSXP, n));

	decode_init(&decode);

	for (i = 0; i < n; i++) {
		data = d->rows[i];
		val = decode_sexp(&decode, &data, &d->schema);
		SET_VECTOR_ELT(ans, i, val);
	}

	if (decode.overflow) {
		warning("Inf introduced by coercion to double-precision range");
	}

	UNPROTECT(1);
	return ans;
}


SEXP simplify_json(SEXP sdata, SEXP stext, SEXP sstringsAsFactors)
{
	SEXP ans;
	const struct json *d = as_json(sdata);
	int overflow;

	switch (d->kind) {
	case CORPUS_DATATYPE_NULL:
	case CORPUS_DATATYPE_BOOLEAN:
		ans = as_logical_json(sdata);
		break;

	case CORPUS_DATATYPE_INTEGER:
		ans = as_integer_json_check(sdata, &overflow);
		if (overflow) {
			ans = as_double_json(sdata);
		}
		break;

	case CORPUS_DATATYPE_REAL:
		ans = as_double_json(sdata);
		break;

	case CORPUS_DATATYPE_TEXT:
		if (LOGICAL(sstringsAsFactors)[0] == TRUE) {
			ans = as_factor_json(sdata);
		} else {
			ans = as_character_json(sdata);
		}
		break;

	case CORPUS_DATATYPE_ARRAY:
		ans = as_list_json(sdata, stext, sstringsAsFactors);
		break;

	default:
		ans = sdata;
		break;
	}

	return ans;
}
