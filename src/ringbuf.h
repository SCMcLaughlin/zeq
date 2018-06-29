
#ifndef RINGBUF_H
#define RINGBUF_H

#include <limits.h>
#include <stdint.h>
#include "cute_func.h"

#define RINGBUF_MIN_PACKETS 2
#define RINGBUF_MAX_PACKETS (INT_MAX / 2)

typedef struct RingBuf RingBuf;

CUTE_FUNC RingBuf* ringbuf_create(uint32_t dataSize, uint32_t slotCount);
CUTE_FUNC RingBuf* ringbuf_destroy(RingBuf* rb);
CUTE_FUNC int ringbuf_push(RingBuf* rb, const void* data);
CUTE_FUNC int ringbuf_pop(RingBuf* rb, void* data);

#endif/*RINGBUF_H*/
