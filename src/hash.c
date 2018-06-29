
#include <stdint.h>
#include "hash.h"
#include "zeq_bit.h"

uint32_t hash_string(const char* str, uint32_t len)
{
    uint32_t h = len;
    uint32_t step = (len >> 5) + 1;
    uint32_t i;
    
    for (i = len; i >= step; i -= step) {
        h = h ^ ((h << 5) + (h >> 2) + (str[i - 1]));
    }
    
    return h;
}

typedef union hashnum_t {
    double      real;
    int64_t     integer;
    struct {
        uint32_t    low;
        uint32_t    high;
    };
} hashnum_t;

#define HASH_ROT1 14
#define HASH_ROT2 5
#define HASH_ROT3 13

static uint32_t hash_rotate(uint32_t lo, uint32_t hi)
{
    hi <<= 1;
    lo ^= hi; hi = bit_rotate(hi, HASH_ROT1);
    lo -= hi; hi = bit_rotate(hi, HASH_ROT2);
    hi ^= lo; hi -= bit_rotate(lo, HASH_ROT3);
    return hi;
}

uint32_t hash_double(double value)
{
    hashnum_t v;
    v.real = value;
    return hash_rotate(v.low, v.high);
}

uint32_t hash_int64(int64_t value)
{
    hashnum_t v;
    v.integer = value;
    return hash_rotate(v.low, v.high);
}
