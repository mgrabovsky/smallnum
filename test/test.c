#include <stdbool.h>
#include <stdio.h>

#include "../number.h"

#define TESTING
#define TESTING_MAIN
#include "../lib/testing/testing.h"

TEST(init__uninitialized) {
    SN a;

    sn_init(&a);

    ASSERT(a.size == 1);
    ASSERT(a.blocks[0] == 0);
    ASSERT(!a.neg);
}

TEST(init__initialized) {
    SN a = { (sn_word *)666, 5, true };

    ASSERT(sn_init(&a) == &a);
    ASSERT(a.size == 1);
    ASSERT(a.blocks[0] == 0);
    ASSERT(!a.neg);
}

TEST(new__01) {
    SN *a = sn_new();

    ASSERT(a);
    ASSERT(a->size == 1);
    ASSERT(a->blocks[0] == 0);
    ASSERT(!a->neg);
}

TEST(copy__01) {
    sn_word words[] = { 0xdeadbeef, 0x2666 };
    SN b, a = { words, 2, true };

    ASSERT(sn_copy(&b, &a) == &b);
    ASSERT(b.size == 2);
    ASSERT(b.blocks[0] == words[0] && b.blocks[1] == words[1]);
    ASSERT(b.neg);
}

TEST(duplicate__01) {
    sn_word words[] = { 0xd00db00b, 0x1948 };
    SN a = { words, 2, false };

    SN *b = sn_duplicate(&a);
    ASSERT(b);
    ASSERT(b->size == 2);
    ASSERT(b->blocks[0] == words[0] && b->blocks[1] == words[1]);
    ASSERT(!b->neg);
}

TEST(swap__01) {
    sn_word a_words[] = { 0xfaceface };
    sn_word b_words[] = { 0xdeaddead, 0xff00ff00};
    SN a = { a_words, 1, false };
    SN b = { b_words, 2, true };

    sn_swap(&a, &b);

    ASSERT(a.size == 2);
    ASSERT(a.blocks[0] == b_words[0] && a.blocks[1] == b_words[1]);
    ASSERT(a.neg);

    ASSERT(b.size == 1);
    ASSERT(b.blocks[0] == a_words[0]);
    ASSERT(!b.neg);
}

TEST(free__01) {
    // TODO
    ASSERT(true);
}

TEST(clear__01) {
    // TODO
    ASSERT(true);
}

TEST(clear_free__01) {
    // TODO
    ASSERT(true);
}

TEST(zero__one_word) {
    sn_word words[] = { 0x49494949 };
    SN a = { words, 1, true };

    sn_zero(&a);
    ASSERT(a.size == 1);
    ASSERT(a.blocks[0] == 0);
    ASSERT(!a.neg);
}

TEST(zero__multiple_words) {
    sn_word *words = malloc(3 * sizeof(*words));
    ASSERT(words);
    words[0] = 0x49494949;
    words[1] = 0x50055005;
    words[2] = 0xfafafafa;

    SN a = { words, 3, false };

    sn_zero(&a);
    ASSERT(a.size == 1);
    ASSERT(a.blocks[0] == 0);
    ASSERT(!a.neg);
}

TEST(one__one_word) {
    sn_word words[] = { 0x66666666 };
    SN a = { words, 1, true };

    sn_one(&a);
    ASSERT(a.size == 1);
    ASSERT(a.blocks[0] == 1);
    ASSERT(!a.neg);
}

TEST(one__multiple_words) {
    sn_word *words = malloc(3 * sizeof(*words));
    ASSERT(words);
    words[0] = 0x11110000;
    words[1] = 0x55511000;
    words[2] = 0xefefefef;

    SN a = { words, 3, false };

    sn_one(&a);
    ASSERT(a.size == 1);
    ASSERT(a.blocks[0] == 1);
    ASSERT(!a.neg);
}

TEST(test01) {
    SN *m, *n, *p, *q;

    m = sn_new();
    n = sn_new();
    p = sn_new();
    q = sn_new();

    sn_one(m);
    sn_zero(n);
    sn_zero(p);

    /* Copmarisons */
    ASSERT(sn_cmp(m, n) > 0);
    ASSERT(sn_ucmp(m, n) > 0);
    m->neg = true;
    ASSERT(sn_cmp(m, n) < 0);
    ASSERT(sn_ucmp(m, n) > 0);
    m->neg = false;

    ASSERT(sn_cmp(n, p) == 0);
    ASSERT(sn_ucmp(n, p) == 0);

    /* Addition of non-negative numbers */
    sn_add(q, m, n);
    ASSERT(sn_cmp(q, m) == 0);
    ASSERT(sn_is_one(q));

    m->blocks[0] = 0xffffffff;
    n->blocks[0] = 0xffffffff;
    sn_add(q, m, n);
    ASSERT(q->size == 2);
    ASSERT(q->blocks[0] == 4294967294 && q->blocks[1] == 1);
    ASSERT(sn_cmp(q, m) > 0);

    /* Serialisation & deserialisation */
    uint8_t bin[8];
    sn_sn2bin(q, bin);
    SN *r = sn_bin2sn(bin, 8, NULL);
    ASSERT(sn_cmp(r, q) == 0);
    sn_free(r);

    sn_free(m);
    sn_free(n);
    sn_free(p);
    sn_free(q);
}

/* vim: set et sw=4: */
