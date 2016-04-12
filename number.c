#include <assert.h>
#include <endian.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "number.h"

static bool sn_valid__(const SN *);
static SN *sn_resize__(SN * const, size_t);

/* =============================================================================
 * Initialization and cleanup functions
 * =============================================================================
 */

/**
 * Initialize number and set it to zero.
 */
SN *sn_init(SN * const num) {
    assert(num);

    num->blocks = calloc(1, sizeof(*num->blocks));
    if (!num->blocks) {
        return NULL;
    }

    num->size = 1;
    num->neg  = false;

    return num;
}

SN *sn_new(void) {
    SN *ret = malloc(sizeof(*ret));
    if (!ret)
        return NULL;

    return sn_init(ret);
}

SN *sn_copy(SN * const restrict dst, const SN * restrict src) {
    assert(dst && src && sn_valid__(src));
    assert(dst != src);

    *dst = *src;
    dst->blocks = malloc(src->size * sizeof(*src->blocks));
    if (!dst->blocks) {
        return NULL;
    }

    memcpy(dst->blocks, src->blocks, src->size * sizeof(*src->blocks));

    return dst;
}

SN *sn_duplicate(const SN *src) {
    assert(src);

    SN *dup = malloc(sizeof(*dup));
    if (!dup) {
        return NULL;
    }

    return sn_copy(dup, src);
}

void sn_swap(SN * const restrict a, SN * const restrict b) {
    SN tmp;
    tmp = *a;
    *a  = *b;
    *b  = tmp;
}

void sn_free(SN *num) {
    assert(num && num->blocks);

    free(num->blocks);
    free(num);
    num = NULL;
}

void sn_clear(SN * const num) {
    assert(num);

    memset(num->blocks, 0, sn_num_bytes(num));
    sn_zero(num);

    num->size = 1;
    num->neg  = false;
}

void sn_clear_free(SN * const num) {
    assert(num);

    sn_clear(num);
    sn_free(num);
}

void sn_zero(SN * const num) {
    assert(num && sn_valid__(num));

    if (num->size > 1) {
        sn_resize__(num, 1);
    }

    num->neg       = false;
    num->blocks[0] = 0;
}

void sn_one(SN * const num) {
    assert(num && sn_valid__(num));

    if (num->size > 1) {
        sn_resize__(num, 1);
    }

    num->neg       = false;
    num->blocks[0] = 1;
}

/* =============================================================================
 * Comparisons and tests
 * =============================================================================
 */

int sn_ucmp(const SN *a, const SN *b) {
    assert(a && b);

    if (a->size > b->size) {
        return 1;
    } else if (a->size < b->size) {
        return -1;
    } else {
        for (size_t i = 0; i < a->size; ++i) {
            if (a->blocks[i] > b->blocks[i])
                return 1;
            else if (a->blocks[i] < b->blocks[i])
                return -1;
        }
    }

    return 0;
}

int sn_cmp(const SN *a, const SN *b) {
    assert(a && b);

    if (a->neg ^ b->neg) {
        return (- a->neg) | b->neg;
    }

    int ordering = sn_ucmp(a, b);
    if (a->neg) {
        return -ordering;
    } else {
        return ordering;
    }
}

bool sn_is_zero(const SN *num) {
    assert(num);

    return num->size == 1 && num->blocks[0] == 0;
}

bool sn_is_one(const SN *num) {
    assert(num);

    return num->size == 1 && num->blocks[0] == 1;
}

bool sn_is_negative(const SN *num) {
    assert(num);

    return num->neg;
}

void sn_set_negative(SN * const num, bool neg) {
    assert(num);

    num->neg = neg;
}

bool sn_is_odd(const SN *num) {
    assert(num);

    return num->blocks[0] & 1;
}

bool sn_is_even(const SN *num) {
    assert(num);

    return !(num->blocks[0] & 1);
}

/* **********************************************************************************
 * ????
 */

size_t sn_num_bytes(const SN *num) {
    assert(num);

    /* FIXME: Not precise */
    return num->size * sizeof(*num->blocks);
}

size_t sn_num_bits(const SN *num) {
    assert(num);

    /* FIXME */
    return 0;
}

/* **********************************************************************************
 * Utilities
 */

static bool sn_valid__(const SN *num) {
    return num && num->blocks && num->size;
}

static SN *sn_resize__(SN * const num, size_t new_size) {
    assert(num && sn_valid__(num) && new_size > 0);

    if (num->size == new_size)
        return num;

    num->blocks = realloc(num->blocks, new_size * sizeof(*num->blocks));
    if (!num->blocks)
        return NULL;

    num->size = new_size;

    return num;
}

/* **********************************************************************************
 * Basic arithmetic operations
 */

SN *sn_add(SN * const restrict res, const SN *a, const SN *b) {
    /* FIXME: Handle negatives */
    /* addition
     * --------
     *  a +  b =   a + b
     * -a +  b =   b - a  ↓
     *  a + -b =   a - b  ↓
     * -a + -b = -(a + b)
     *
     * subtraction
     * -----------
     *  a -  b =   a - b
     * -a -  b = -(a + b) ↑
     *  a - -b =   a + b  ↑
     * -a - -b =   b - a
     */
    assert(res && a && b);
    assert(res->blocks != a->blocks && res->blocks != b->blocks);

    /* Preliminary size */
    size_t sum_size = max(a->size, b->size);
    if (!sn_resize__(res, sum_size))
        return NULL;

    bool overflow = false;
    sn_word tmp  = 0;

    for (size_t i = 0; i < sum_size; ++i) {
        if (i >= a->size) {
            tmp            = b->blocks[i] + overflow;
            res->blocks[i] = tmp;
            overflow       = (tmp < b->blocks[i]);
        } else if (i >= b->size) {
            tmp            = a->blocks[i] + overflow;
            res->blocks[i] = tmp;
            overflow       = (tmp < a->blocks[i]);
        } else {
            tmp            = a->blocks[i] + b->blocks[i] + overflow;
            res->blocks[i] = tmp;
            overflow       = (tmp < a->blocks[i] || tmp < b->blocks[i]);
        }
    }

    if (overflow) {
        if (!sn_resize__(res, sum_size + 1))
            return NULL;
        res->blocks[sum_size] = overflow;
    }

    return res;
}

/* **********************************************************************************
 * Printing and loading
 */

size_t sn_sn2bin(const SN *num, uint8_t * const dst) {
    assert(num && dst);

    sn_word b;
    for (size_t i = 0; i < num->size; ++i) {
        b = num->blocks[num->size - i - 1];
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        dst[4*i]   = (b & 0xff000000) >> 24;
        dst[4*i+1] = (b & 0x00ff0000) >> 16;
        dst[4*i+2] = (b & 0x0000ff00) >> 8;
        dst[4*i+3] =  b & 0x000000ff;
#else
        dst[4*i]   =  b & 0x000000ff;
        dst[4*i+1] = (b & 0x0000ff00) >> 8;
        dst[4*i+2] = (b & 0x00ff0000) >> 16;
        dst[4*i+3] = (b & 0xff000000) >> 24;
#endif // __BYTE_ORDER__ 
    }

    return sn_num_bytes(num);
}

SN *sn_bin2sn(const uint8_t *src, size_t length, SN *res) {
    assert(src && length > 0);

    if (!res) {
        res = sn_new();
        if (!sn_resize__(res, 1 + (length - 1) / sizeof(*res->blocks))) {
            sn_free(res);
            return NULL;
        }
    } else if (!sn_resize__(res, 1 + (length - 1) / sizeof(*res->blocks))) {
        return NULL;
    }

    sn_word *b;
    for (size_t i = 0; i < length; ++i) {
        if (i % 4 == 0) {
            b  = &res->blocks[res->size - i / 4 - 1];
            *b = 0;
        }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        *b = (*b & ~(0xff << (24 - 8 * (i % 4)))) | src[i] << (24 - 8 * (i % 4));
#else
        *b = (*b & ~(0xff << (8 * (i % 4)))) | src[i] << (8 * (i % 4));
#endif // __BYTE_ORDER__ 
    }

    return res;
}

/* vim: set et sw=4: */
