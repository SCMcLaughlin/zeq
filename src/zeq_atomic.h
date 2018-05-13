
#ifndef ZEQ_ATOMIC_H
#define ZEQ_ATOMIC_H

#include <stdbool.h>
#include <stdint.h>

typedef int8_t atomic8_t;
typedef int16_t atomic16_t;
typedef int32_t atomic32_t;

void atomic8_set(atomic8_t* a, int8_t val);
int8_t atomic8_get(atomic8_t* a);
int8_t atomic8_add(atomic8_t* a, int8_t val); /* Returns new value */
int8_t atomic8_sub(atomic8_t* a, int8_t val); /* Returns new value */
bool atomic8_cmp_xchg(atomic8_t* a, int8_t expected, int8_t desired);

void atomic16_set(atomic16_t* a, int16_t val);
int16_t atomic16_get(atomic16_t* a);
int16_t atomic16_add(atomic16_t* a, int16_t val); /* Returns new value */
int16_t atomic16_sub(atomic16_t* a, int16_t val); /* Returns new value */
bool atomic16_cmp_xchg(atomic16_t* a, int16_t expected, int16_t desired);

void atomic32_set(atomic32_t* a, int32_t val);
int32_t atomic32_get(atomic32_t* a);
int32_t atomic32_add(atomic32_t* a, int32_t val); /* Returns new value */
int32_t atomic32_sub(atomic32_t* a, int32_t val); /* Returns new value */
bool atomic32_cmp_xchg(atomic32_t* a, int32_t expected, int32_t desired);

#endif/*ZEQ_ATOMIC_H*/

