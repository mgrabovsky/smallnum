#ifndef SMALLNUM_NUMBER_H
#define SMALLNUM_NUMBER_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif // !defined min
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif // !defined max

typedef uint32_t sn_word;

/**
 * The allocated number blocks (words) are stored as arrays in little-endian order,
 * i.e. @f$blocks[0] = num \wedge (2^W - 1)@f$, @f$blocks[1] = (num \gg W) \wedge (2^W - 1)@f$, etc.
 * where `num` is the virtual binary number, `**` is the exponentiation operator,
 * and`W` denotes the word size in bits. The endianness of the words themselves
 * depends on the machine.
 *
 * ```
 * +---------------+---------------+--
 * |   :   :   :   |   :   :   :   | ...
 * +---------------+---------------+--
 *  \_____________/ \_____________/
 *      word #1         word #2      etc.
 *     (W bits)        (W bits)
 * ```
 */
typedef struct smallnum {
    sn_word *blocks; /**< Pointer to the beginning of an array of allocated blocks */
    size_t   size; /**< Number of allocated blocks */
    bool     neg; /**< Negative number flag */
} SN;
/*@ type invariant number_size_is_positive(SN a) = a.size > 0; */

/** @defgroup init Creation, initialisation and clean up
 * @{
 */
SN *sn_init(SN * const);
SN *sn_new(void);
SN *sn_copy(SN * const restrict, const SN * restrict);
SN *sn_duplicate(const SN *);
void sn_swap(SN * const restrict, SN * const restrict);
void sn_free(SN *);
void sn_clear(SN * const);
void sn_clear_free(SN * const);
void sn_zero(SN * const);
void sn_one(SN * const);
/* @} */

/** @defgroup cmp Comparisons and tests
 * @{
 */
int sn_ucmp(const SN *, const SN *);
int sn_cmp(const SN *, const SN *);
bool sn_is_zero(const SN *);
bool sn_is_one(const SN *);
bool sn_is_negative(const SN *);
void sn_set_negative(SN * const, bool);
bool sn_is_odd(const SN *);
bool sn_is_even(const SN *);
/* @} */

/** @defgroup utils Utilities
 * @{
 */
size_t sn_num_bytes(const SN *);
size_t sn_num_bits(const SN *);
/* @} */

/** @defgroup arith Basic arithmetic
 * @{
 */
SN *sn_add(SN * const restrict, const SN *, const SN *);
SN *sn_sub(SN * const restrict, const SN *, const SN *);
SN *sn_mul(SN * const restrict, const SN *, const SN *);
/* @} */

/** @defgroup conv Conversion from/to byte strings and character strings
 * @{
 */
size_t sn_sn2bin(const SN *, uint8_t * const);
SN *sn_bin2sn(const uint8_t *, size_t, SN *);
/* @} */

#endif // !defined SMALLNUM_NUMBER_H

/* vim: set et sw=4: */
