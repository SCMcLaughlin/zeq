
#include <stdbool.h>
#include <stdint.h>
#include "zeq_atomic.h"

bool atomic_flag_test_and_set(atomic_flag_t* a)
{
    return __atomic_test_and_set(a, __ATOMIC_SEQ_CST);
}

void atomic_flag_clear(atomic_flag_t* a)
{
    __atomic_clear(a, __ATOMIC_SEQ_CST);
}

void atomic8_set(atomic8_t* a, int8_t val)
{
    __atomic_store_n(a, val, __ATOMIC_SEQ_CST);
}

int8_t atomic8_get(atomic8_t* a)
{
    return __atomic_load_n(a, __ATOMIC_SEQ_CST);
}

int8_t atomic8_add(atomic8_t* a, int8_t val)
{
    return __atomic_add_fetch(a, val, __ATOMIC_SEQ_CST);
}

int8_t atomic8_sub(atomic8_t* a, int8_t val)
{
    return __atomic_sub_fetch(a, val, __ATOMIC_SEQ_CST);
}

bool atomic8_cmp_xchg(atomic8_t* a, int8_t expected, int8_t desired)
{
    return __atomic_compare_exchange_n(a, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

void atomic16_set(atomic16_t* a, int16_t val)
{
    __atomic_store_n(a, val, __ATOMIC_SEQ_CST);
}

int16_t atomic16_get(atomic16_t* a)
{
    return __atomic_load_n(a, __ATOMIC_SEQ_CST);
}

int16_t atomic16_add(atomic16_t* a, int16_t val)
{
    return __atomic_add_fetch(a, val, __ATOMIC_SEQ_CST);
}

int16_t atomic16_sub(atomic16_t* a, int16_t val)
{
    return __atomic_sub_fetch(a, val, __ATOMIC_SEQ_CST);
}

bool atomic16_cmp_xchg(atomic16_t* a, int16_t expected, int16_t desired)
{
    return __atomic_compare_exchange_n(a, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

void atomic32_set(atomic32_t* a, int32_t val)
{
    __atomic_store_n(a, val, __ATOMIC_SEQ_CST);
}

int32_t atomic32_get(atomic32_t* a)
{
    return __atomic_load_n(a, __ATOMIC_SEQ_CST);
}

int32_t atomic32_add(atomic32_t* a, int32_t val)
{
    return __atomic_add_fetch(a, val, __ATOMIC_SEQ_CST);
}

int32_t atomic32_sub(atomic32_t* a, int32_t val)
{
    return __atomic_sub_fetch(a, val, __ATOMIC_SEQ_CST);
}

bool atomic32_cmp_xchg(atomic32_t* a, int32_t expected, int32_t desired)
{
    return __atomic_compare_exchange_n(a, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

