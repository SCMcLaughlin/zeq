
#ifndef ZEQ_BIT_H
#define ZEQ_BIT_H

#include <stdint.h>
#include "zeq_def.h"

#define bit_mask(n) ((1 << (n)) - 1)
#define bit_mask64(n) ((1ULL << (n)) - 1ULL)
#define bit_mask_offset(n, off) (bit_mask(n) << (off))
#define bit_is_pow2(n) ((n) && (((n) & ((n) - 1)) == 0))
#define bit_is_pow2_or_zero(n) ((n == 0) || bit_is_pow2(n))
#define bit_rotate(x, n) (((x)<<(n)) | ((x)>>(-(int)(n)&(8*sizeof(x)-1))))
#define bit_get(val, n) ((val) & (1 << (n)))
#define bit_get64(val, n) ((val) & (1ULL << (n)))
#define bit_set(val, n) ((val) |= (1 << (n)))
#define bit_set64(val, n) ((val) |= (1ULL << (n)))
#define bit_unset(val, n) ((val) &= ~(1 << (n)))
#define bit_unset64(val, n) ((val) &= ~(1ULL << (n)))
#define bit_nth(n) (1 << (n))
#define bit_nth64(n) (1ULL << (n))

ZEQ_INTERFACE uint32_t bit_next_pow2_u32(uint32_t n);
ZEQ_INTERFACE uint32_t bit_pow2_greater_than_u32(uint32_t n);
ZEQ_INTERFACE uint32_t bit_pow2_greater_or_equal_u32(uint32_t n);

#endif/*ZEQ_BIT_H*/
