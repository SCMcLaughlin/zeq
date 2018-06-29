
#ifndef ZEQ_ALLOC_H
#define ZEQ_ALLOC_H

#include <stdlib.h>
#include "zeq_err.h"

#ifdef ZEQ_INJECT_ALLOCATORS
void* zeq_malloc(size_t bytes);
void* zeq_realloc(void* ptr, size_t bytes);
void* zeq_calloc(size_t n, size_t bytes);
#else
# define zeq_malloc malloc
# define zeq_realloc realloc
# define zeq_calloc calloc
#endif

#define realloc_buffer_pow2_else(ptr, count, type, onFail)                                          \
    do {                                                                                            \
    uint32_t grow_buffer_pow2_cap = ((count) == 0) ? 1 : (count) * 2;                               \
    type* grow_buffer_pow2_array = (type*)mfgt_realloc((ptr), sizeof(type) * grow_buffer_pow2_cap); \
    if (!grow_buffer_pow2_array) { onFail; }                                                        \
    (ptr) = grow_buffer_pow2_array;                                                                 \
    } while (0)

#define realloc_buffer_pow2(ptr, count, type) realloc_buffer_pow2_else(ptr, count, type, return ZEQ_ERR_MEMORY)

#endif/*ZEQ_ALLOC_H*/
