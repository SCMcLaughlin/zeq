
#ifndef ZEQ_ATOMIC_H
#define ZEQ_ATOMIC_H

#include <stdbool.h>
#include <stdint.h>
#include "zeq_def.h"

typedef bool atomic_flag_t;
typedef int8_t atomic8_t;
typedef int16_t atomic16_t;
typedef int32_t atomic32_t;

ZEQ_INTERFACE_INLINE bool atomic_flag_test_and_set(atomic_flag_t* a);
ZEQ_INTERFACE_INLINE void atomic_flag_clear(atomic_flag_t* a);

ZEQ_INTERFACE_INLINE void atomic8_set(atomic8_t* a, int8_t val);
ZEQ_INTERFACE_INLINE int8_t atomic8_get(atomic8_t* a);
ZEQ_INTERFACE_INLINE int8_t atomic8_add(atomic8_t* a, int8_t val); /* Returns new value */
ZEQ_INTERFACE_INLINE int8_t atomic8_sub(atomic8_t* a, int8_t val); /* Returns new value */
ZEQ_INTERFACE_INLINE bool atomic8_cmp_xchg(atomic8_t* a, int8_t expected, int8_t desired);

ZEQ_INTERFACE_INLINE void atomic16_set(atomic16_t* a, int16_t val);
ZEQ_INTERFACE_INLINE int16_t atomic16_get(atomic16_t* a);
ZEQ_INTERFACE_INLINE int16_t atomic16_add(atomic16_t* a, int16_t val); /* Returns new value */
ZEQ_INTERFACE_INLINE int16_t atomic16_sub(atomic16_t* a, int16_t val); /* Returns new value */
ZEQ_INTERFACE_INLINE bool atomic16_cmp_xchg(atomic16_t* a, int16_t expected, int16_t desired);

ZEQ_INTERFACE_INLINE void atomic32_set(atomic32_t* a, int32_t val);
ZEQ_INTERFACE_INLINE int32_t atomic32_get(atomic32_t* a);
ZEQ_INTERFACE_INLINE int32_t atomic32_add(atomic32_t* a, int32_t val); /* Returns new value */
ZEQ_INTERFACE_INLINE int32_t atomic32_sub(atomic32_t* a, int32_t val); /* Returns new value */
ZEQ_INTERFACE_INLINE bool atomic32_cmp_xchg(atomic32_t* a, int32_t expected, int32_t desired);

#endif/*ZEQ_ATOMIC_H*/

