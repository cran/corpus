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

#ifndef RCORPUS_H
#define RCORPUS_H

#include <stddef.h>
#include <stdint.h>
#include <Rdefines.h>

#include "corpus/src/table.h"
#include "corpus/src/text.h"
#include "corpus/src/token.h"
#include "corpus/src/symtab.h"
#include "corpus/src/datatype.h"
#include "corpus/src/data.h"

struct data;
struct filebuf;

struct jsondata {
	struct schema schema;
	struct data *rows;
	R_xlen_t nrow;
	int type_id;
	int kind;
};

struct mkchar {
	uint8_t *buf;
	int size;
};

struct decode {
	struct mkchar mkchar;
	int overflow;
};

/* converting text to CHARSXP */
void mkchar_init(struct mkchar *mk);
void mkchar_destroy(struct mkchar *mk);
SEXP mkchar_get(struct mkchar *mk, const struct text *text);

/* converting data to R values */
void decode_init(struct decode *d);
void decode_clear(struct decode *d);
void decode_destroy(struct decode *d);

int decode_logical(struct decode *d, const struct data *val);
int decode_integer(struct decode *d, const struct data *val);
double decode_real(struct decode *d, const struct data *val);
SEXP decode_charsxp(struct decode *d, const struct data *val);
SEXP decode_sexp(struct decode *d, const struct data *val,
		 const struct schema *s);

/* logging */
SEXP logging_off(void);
SEXP logging_on(void);

/* data set */
SEXP alloc_jsondata(SEXP sfilebuf, SEXP sfield, SEXP srows);
int is_jsondata(SEXP data);
struct jsondata *as_jsondata(SEXP data);

SEXP as_integer_jsondata(SEXP data);
SEXP as_double_jsondata(SEXP data);
SEXP as_list_jsondata(SEXP data);
SEXP as_logical_jsondata(SEXP data);
SEXP as_text_jsondata(SEXP data);
SEXP dim_jsondata(SEXP data);
SEXP length_jsondata(SEXP data);
SEXP names_jsondata(SEXP data);
SEXP print_jsondata(SEXP data);
SEXP datatype_jsondata(SEXP data);
SEXP datatypes_jsondata(SEXP data);
SEXP simplify_jsondata(SEXP data);
SEXP subscript_jsondata(SEXP data, SEXP i);
SEXP subset_jsondata(SEXP data, SEXP i, SEXP j);

/* data */
SEXP scalar_data(const struct data *d, const struct schema *s,
		 int *overflowptr);

/* file buffer */
SEXP alloc_filebuf(SEXP file);
int is_filebuf(SEXP sbuf);
struct filebuf *as_filebuf(SEXP sbuf);

/* text (core) */
SEXP alloc_text(SEXP sources, SEXP source, SEXP row, SEXP start, SEXP stop);
int is_text(SEXP text);
struct text *as_text(SEXP text, R_xlen_t *lenptr);
SEXP as_text_character(SEXP text);

SEXP alloc_na_text(void);
SEXP coerce_text(SEXP x);
SEXP length_text(SEXP text);
SEXP names_text(SEXP text);
SEXP subset_text_handle(SEXP handle, SEXP i);
SEXP as_character_text(SEXP text);
SEXP is_na_text(SEXP text);
SEXP anyNA_text(SEXP text);

/* text filter */
int is_text_filter(SEXP filter);
int text_filter_type_kind(SEXP filter);
int text_filter_drop_empty(SEXP filter);
const char *text_filter_stemmer(SEXP filter);

/* text processing */
SEXP sentences_text(SEXP x);
SEXP tokens_text(SEXP x, SEXP filter);
SEXP word_counts_text(SEXP x, SEXP filter);

/* data schema */
SEXP alloc_schema(void);
int is_schema(SEXP schema);
struct schema *as_schema(SEXP schema);

/* json values */
SEXP read_ndjson(SEXP file);

/* internal utility functions */
int findListElement(SEXP list, const char *str);
SEXP getListElement(SEXP list, const char *str);

#endif /* RCORPUS_H */
