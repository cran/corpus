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

#include <check.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../src/table.h"
#include "../src/text.h"
#include "../src/textset.h"
#include "../src/tree.h"
#include "../src/typemap.h"
#include "../src/symtab.h"
#include "../src/wordscan.h"
#include "../src/filter.h"
#include "../src/census.h"
#include "testutil.h"

#define IGNORE_EMPTY CORPUS_FILTER_IGNORE_EMPTY

#define ID_EOT	(-1)
#define ID_NONE (-2)
#define TERM_EOT ((const struct corpus_text *)&term_eot)
#define TERM_NONE ((const struct corpus_text *)&term_none)

static struct corpus_text term_eot, term_none;
static struct corpus_filter filter;
static int has_filter;


static void setup_filter(void)
{
	setup();
	has_filter = 0;
	term_eot.ptr = (uint8_t *)"<eot>";
	term_eot.attr = strlen("<eot>");
	term_none.ptr = (uint8_t *)"<none>";
	term_none.attr = strlen("<none>");
}


static void teardown_filter(void)
{
	if (has_filter) {
		corpus_filter_destroy(&filter);
		has_filter = 0;
	}
	teardown();
}


static void init(const char *stemmer, int flags)
{
	int type_kind = (CORPUS_TYPE_COMPAT | CORPUS_TYPE_CASEFOLD
			 | CORPUS_TYPE_DASHFOLD | CORPUS_TYPE_QUOTFOLD
			 | CORPUS_TYPE_RMCC | CORPUS_TYPE_RMDI
			 | CORPUS_TYPE_RMWS);

	ck_assert(!has_filter);
	ck_assert(!corpus_filter_init(&filter, type_kind, stemmer, flags));
	has_filter = 1;
}


static void start(const struct corpus_text *text)
{
	ck_assert(!corpus_filter_start(&filter, text));
}

static int next_id(void)
{
	int term_id, type_id;
	int has = corpus_filter_advance(&filter, &term_id);

	ck_assert(!filter.error);

	if (has) {
		if (term_id < 0) {
			return ID_NONE;
		}
		ck_assert_int_lt(term_id, filter.nterm);
		type_id = filter.type_ids[term_id];
		ck_assert_int_le(0, type_id);
		ck_assert_int_lt(type_id, filter.symtab.ntype);
		return term_id;
	} else {
		return ID_EOT;
	}
}


static const struct corpus_text *next_term(void)
{
	int term_id = next_id();
	int type_id;

	switch (term_id) {
	case ID_EOT:
		return TERM_EOT;
	case ID_NONE:
		return TERM_NONE;
	default:
		type_id = filter.type_ids[term_id];
		return &filter.symtab.types[type_id].text;
	}
}


START_TEST(test_basic)
{
	init(NULL, IGNORE_EMPTY);

	start(T("A rose is a rose is a rose."));
	assert_text_eq(next_term(), T("a"));
	assert_text_eq(next_term(), T("rose"));
	assert_text_eq(next_term(), T("is"));
	assert_text_eq(next_term(), T("a"));
	assert_text_eq(next_term(), T("rose"));
	assert_text_eq(next_term(), T("is"));
	assert_text_eq(next_term(), T("a"));
	assert_text_eq(next_term(), T("rose"));
	assert_text_eq(next_term(), T("."));
	assert_text_eq(next_term(), TERM_EOT);
}
END_TEST


START_TEST(test_basic_census)
{
	struct corpus_census census;
	int term_id;

	ck_assert(!corpus_census_init(&census));

	init(NULL, IGNORE_EMPTY);
	start(T("A rose is a rose is a rose."));

	while ((term_id = next_id()) != ID_EOT) {
		if (term_id == ID_NONE) {
			continue;
		}

		ck_assert(!corpus_census_add(&census, term_id, 1));
	}

	ck_assert(!corpus_census_sort(&census));

	ck_assert_int_eq(census.nitem, 4);
	ck_assert_int_eq(census.items[0], 0); // a
	ck_assert_double_eq(census.weights[0], 3);
	ck_assert_int_eq(census.items[1], 1); // rose
	ck_assert_double_eq(census.weights[1], 3);
	ck_assert_int_eq(census.items[2], 2); // is
	ck_assert_double_eq(census.weights[2], 2);
	ck_assert_int_eq(census.items[3], 3); // .
	ck_assert_double_eq(census.weights[3], 1);

	corpus_census_destroy(&census);
}
END_TEST


Suite *filter_suite(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("filter");

	tc = tcase_create("basic");
	tcase_add_checked_fixture(tc, setup_filter, teardown_filter);
        tcase_add_test(tc, test_basic);
        tcase_add_test(tc, test_basic_census);
	suite_add_tcase(s, tc);

	return s;
}


int main(void)
{
	int nfail;
	Suite *s;
	SRunner *sr;

	s = filter_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	nfail = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nfail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
