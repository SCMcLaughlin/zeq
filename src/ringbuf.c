
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cute_atomic.h"
#include "cute_byte.h"
#include "cute_err.h"
#include "cute_platform.h"
#include "ringbuf.h"

#ifdef CUTE_COMPILER_MSVC
/* data[0]: "nonstandard extension used: zero-sized array in struct/union" */
# pragma warning(disable: 4200)
#endif

typedef struct {
    byte    data[0];
} RingBlock;

struct RingBuf {
    int         readStart;
    int         readEnd;
    int         writeStart;
    int         writeEnd;
    uint32_t    dataSize;
    int         slotCount;
    byte        data[0];
};

#define ringbuf_get_block(rb, n) ((RingBlock*)(&((rb)->data[(rb)->dataSize * (n)])))

RingBuf* ringbuf_create(uint32_t dataSize, uint32_t slotCount)
{
    RingBuf* rb;

    assert(slotCount >= RINGBUF_MIN_PACKETS && slotCount <= RINGBUF_MAX_PACKETS);

    rb = (RingBuf*)malloc(sizeof(RingBuf) + (dataSize * slotCount));
    if (!rb) return NULL;
    
    rb->dataSize = dataSize;
    rb->slotCount = (int)slotCount;
    rb->readStart = 0;
    rb->readEnd = 0;
    rb->writeStart = 0;
    rb->writeEnd = 0;
    return rb;
}

RingBuf* ringbuf_destroy(RingBuf* rb)
{
    if (rb) {
        free(rb);
    }

    return NULL;
}

static void ringbuf_push_impl(RingBuf* rb, RingBlock* block, const void* p)
{
    if (p) memcpy(&block->data[0], p, rb->dataSize);
    
    rb->writeStart++;
    if (rb->writeStart == rb->slotCount)
        rb->writeStart = 0;
}

int ringbuf_push(RingBuf* rb, const void* p)
{
    int index = atomic32_get(&rb->writeEnd);
    int next = index + 1;
    
    if (next == rb->slotCount)
        next = 0;
    
    if (next == rb->readStart)
        return CUTE_ERR_BOUNDS;
    
    rb->writeEnd = next;
    
    ringbuf_push_impl(rb, ringbuf_get_block(rb, index), p);
    return CUTE_OK;
}

static void ringbuf_pop_impl(RingBuf* rb, RingBlock* block, void* p)
{
    if (p) memcpy(p, &block->data[0], rb->dataSize);
    
    rb->readStart++;
    if (rb->readStart == rb->slotCount)
        rb->readStart = 0;
}

int ringbuf_pop(RingBuf* rb, void* p)
{
    int index = rb->readEnd;
    
    if (index == rb->writeStart)
        return CUTE_ERR_AGAIN;
    
    rb->readEnd = index + 1;
    if (rb->readEnd == rb->slotCount)
        rb->readEnd = 0;
    
    ringbuf_pop_impl(rb, ringbuf_get_block(rb, index), p);
    return CUTE_OK;
}
