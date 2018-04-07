#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <cmocka.h>

#include "../number.h"

/* Initialization */
static void init__unitialized(void **state) {
    SN a;
    SN *b = sn_init(&a);

    assert_non_null(b);
    assert_ptr_equal(b, &a);

    assert_int_equal(a.size, 1);
    assert_int_equal(a.blocks[0], 0);
    assert_false(a.neg);

    free(a.blocks);
}

static void init__initialized(void **state) {
    SN  a = { (sn_word *)666, 5, true };
    SN *b = sn_init(&a);

    assert_non_null(b);
    assert_ptr_equal(b, &a);

    assert_int_equal(a.size, 1);
    assert_int_equal(a.blocks[0], 0);
    assert_false(a.neg);

    free(a.blocks);
}

static void new__basic(void **state) {
    SN *a = sn_new();

    assert_non_null(a);
    assert_int_equal(a->size, 1);
    assert_int_equal(a->blocks[0], 0);
    assert_false(a->neg);

    free(a->blocks);
    free(a);
}

/* Copying */
static void copy__01(void **state) {
    sn_word words[] = { 0xdeadbeef, 0x2666 };
    SN b, a = { words, 2, true };

    SN *c = sn_copy(&b, &a);

    assert_ptr_equal(c, &b);
    assert_int_equal(b.size, 2);
    assert_int_equal(b.blocks[0], words[0]);
    assert_int_equal(b.blocks[1], words[1]);
    assert_true(b.neg);

    free(b.blocks);
}

static void duplicate__01(void **state) {
    sn_word words[] = { 0xd00db00b, 0x1948 };
    SN a = { words, 2, false };

    SN *b = sn_duplicate(&a);

    assert_non_null(b);
    assert_int_equal(b->size, 2);
    assert_int_equal(b->blocks[0], words[0]);
    assert_int_equal(b->blocks[1], words[1]);
    assert_false(b->neg);

    free(b->blocks);
    free(b);
}

/* Swapping */
static void swap__01(void **state) {
    sn_word a_words[] = { 0xfaceface };
    sn_word b_words[] = { 0xdeaddead, 0xff00ff00};
    SN a = { a_words, 1, false };
    SN b = { b_words, 2, true };

    sn_swap(&a, &b);

    assert_int_equal(a.size, 2);
    assert_int_equal(a.blocks[0], b_words[0]);
    assert_int_equal(a.blocks[1], b_words[1]);
    assert_true(a.neg);

    assert_int_equal(b.size, 1);
    assert_int_equal(b.blocks[0], a_words[0]);
    assert_false(b.neg);
}

/* Cleanup */
static void free__01(void **state) {
    // TODO
    assert_true(true);
}

static void clear__01(void **state) {
    // TODO
    assert_true(true);
}

static void clear_free__01(void **state) {
    // TODO
    assert_true(true);
}

/* Resetting */
static void zero__one_word(void **state) {
    sn_word words[] = { 0x49494949 };
    SN a = { words, 1, true };

    sn_zero(&a);
    assert_int_equal(a.size, 1);
    assert_int_equal(a.blocks[0], 0);
    assert_false(a.neg);
}

static void zero__multiple_words(void **state) {
    sn_word *words = malloc(3 * sizeof(*words));
    assert_non_null(words);

    words[0] = 0x49494949;
    words[1] = 0x50055005;
    words[2] = 0xfafafafa;

    SN a = { words, 3, false };
    sn_zero(&a);

    assert_int_equal(a.size, 1);
    assert_int_equal(a.blocks[0], 0);
    assert_false(a.neg);

    free(a.blocks);
}

static void one__one_word(void **state) {
    sn_word words[] = { 0x66666666 };
    SN a = { words, 1, true };

    sn_one(&a);
    assert_int_equal(a.size, 1);
    assert_int_equal(a.blocks[0], 1);
    assert_false(a.neg);
}

static void one__multiple_words(void **state) {
    sn_word *words = malloc(3 * sizeof(*words));
    assert_non_null(words);

    words[0] = 0x11110000;
    words[1] = 0x55511000;
    words[2] = 0xefefefef;

    SN a = { words, 3, false };
    sn_one(&a);

    assert_int_equal(a.size, 1);
    assert_int_equal(a.blocks[0], 1);
    assert_false(a.neg);

    free(a.blocks);
}

/* Addition */
static void add__size_1_nonoverflow(void **state) {
    SN *m = sn_new();
    SN *n = sn_new();
    SN *res = sn_new();

    m->blocks[0] = 0xfaffffff;
    n->blocks[0] = 1;

    sn_add(res, m, n);

    assert_int_equal(res->size, 1);
    assert_int_equal(res->blocks[0], 0xfb000000);

    sn_free(m);
    sn_free(n);
    sn_free(res);
}

static void add__size_1_overflow(void **state) {
    SN *m = sn_new();
    SN *n = sn_new();
    SN *res = sn_new();

    m->blocks[0] = 0xffffffff;
    n->blocks[0] = 1;

    sn_add(res, m, n);

    assert_int_equal(res->size, 2);
    assert_int_equal(res->blocks[0], 0);
    assert_int_equal(res->blocks[1], 1);

    sn_free(m);
    sn_free(n);
    sn_free(res);
}

static void add__size_2_overflow(void **state) {
    SN *m = sn_new();
    SN *n = sn_new();
    SN *res = sn_new();

    m->blocks[0] = 0xffffffff;
    n->blocks[0] = 1;

    sn_add(res, m, n);

    assert_int_equal(res->size, 2);
    assert_int_equal(res->blocks[0], 0);
    assert_int_equal(res->blocks[1], 1);

    sn_free(m);
    sn_free(n);
    sn_free(res);
}

/* Subtraction */
static void sub__size_1_nonunderflow(void **state) {
    SN *m = sn_new();
    SN *n = sn_new();
    SN *res = sn_new();

    m->blocks[0] = 0xfaffffff;
    n->blocks[0] = 1;

    sn_sub(res, m, n);

    assert_int_equal(res->size, 1);
    assert_int_equal(res->blocks[0], 0xfafffffe);
    assert_false(res->neg);

    sn_free(m);
    sn_free(n);
    sn_free(res);
}

static void sub__size_2_underflow(void **state) {
    SN *m = sn_new();
    SN *n = sn_new();
    SN *res = sn_new();

    m->blocks[0] = 0xffffffff;
    n->blocks[0] = 1;

    sn_add(res, m, n);

    assert_int_equal(res->size, 2);
    assert_int_equal(res->blocks[0], 0);
    assert_int_equal(res->blocks[1], 1);
    assert_false(res->neg);

    n->blocks[0] = 2;
    sn_swap(res, m);
    sn_sub(res, m, n);

    assert_int_equal(res->size, 2);
    assert_int_equal(res->blocks[0], 0xfffffffe);
    assert_int_equal(res->blocks[1], 0);
    assert_false(res->neg);

    sn_free(m);
    sn_free(n);
    sn_free(res);
}

static void sub__size_3_underflow(void **state) {
    SN *m = sn_new();
    SN *n = sn_new();
    SN *res = sn_new();

    m->blocks[0] = 0xffffffff;
    n->blocks[0] = 1;

    sn_add(res, m, n);
    sn_swap(res, m);
    m->blocks[0] = m->blocks[1] = 0xffffffff;
    sn_add(res, m, n);

    assert_int_equal(res->size, 3);
    assert_int_equal(res->blocks[0], 0);
    assert_int_equal(res->blocks[1], 0);
    assert_int_equal(res->blocks[2], 1);
    assert_false(res->neg);

    n->blocks[0] = 2;
    sn_swap(res, m);
    sn_sub(res, m, n);

    assert_int_equal(res->size, 3);
    assert_int_equal(res->blocks[0], 0xfffffffe);
    assert_int_equal(res->blocks[1], 0xffffffff);
    assert_int_equal(res->blocks[2], 0);
    assert_false(res->neg);

    sn_free(m);
    sn_free(n);
    sn_free(res);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(init__unitialized),
        cmocka_unit_test(init__initialized),
        cmocka_unit_test(new__basic),
        cmocka_unit_test(copy__01),
        cmocka_unit_test(duplicate__01),
        cmocka_unit_test(swap__01),
        cmocka_unit_test(free__01),
        cmocka_unit_test(clear__01),
        cmocka_unit_test(clear_free__01),
        cmocka_unit_test(zero__one_word),
        cmocka_unit_test(zero__multiple_words),
        cmocka_unit_test(one__one_word),
        cmocka_unit_test(one__multiple_words),
        cmocka_unit_test(add__size_1_nonoverflow),
        cmocka_unit_test(add__size_1_overflow),
        cmocka_unit_test(add__size_2_overflow),
        cmocka_unit_test(sub__size_1_nonunderflow),
        cmocka_unit_test(sub__size_2_underflow),
        cmocka_unit_test(sub__size_3_underflow),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

/* vim: set et sw=4: */
