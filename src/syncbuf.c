
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "syncbuf.h"
#include "zeq_alloc.h"
#include "zeq_atomic.h"
#include "zeq_byte.h"
#include "zeq_def.h"
#include "zeq_err.h"
#include "zeq_platform.h"
#include "zeq_semaphore.h"

#ifdef ZEQ_COMPILER_MSVC
/* data[0]: "nonstandard extension used: zero-sized array in struct/union" */
# pragma warning(disable: 4200)
#endif

typedef struct SyncBlock {
    int         opcode;
    atomic8_t   hasBeenRead;
    atomic8_t   hasBeenWritten;
    uint16_t    padding;
    byte        data[0];
} SyncBlock;

struct SyncBuf {
    atomic32_t  readStart;
    atomic32_t  readEnd;
    atomic32_t  writeStart;
    atomic32_t  writeEnd;
    uint32_t    dataSize;
    int         slotCount;
    union {
        Semaphore   sem;
        uint64_t    padding;
    };
    byte        data[0];
};

#define syncbuf_get_block(sb, n) ((SyncBlock*)(&((sb)->data[(sizeof(SyncBlock) + (sb)->dataSize) * (n)])))

SyncBuf* syncbuf_create(uint32_t slotCount, uint32_t dataSize)
{
    SyncBuf* sb;

    assert(slotCount >= SYNCBUF_MIN_PACKETS && slotCount <= SYNCBUF_MAX_PACKETS);
    
    if (slotCount < SYNCBUF_MIN_PACKETS || slotCount > SYNCBUF_MAX_PACKETS)
        return NULL;

    sb = (SyncBuf*)zeq_calloc(1, sizeof(SyncBuf) + (dataSize + sizeof(SyncBlock)) * slotCount);
    if (!sb) return NULL;
    
    sb->dataSize = dataSize;
    sb->slotCount = (int)slotCount;
    
    if (semaphore_init(&sb->sem)) {
        free(sb);
        return NULL;
    }
    return sb;
}

SyncBuf* syncbuf_destroy(SyncBuf* sb)
{
    if (sb) {
        semaphore_deinit(&sb->sem);
        free(sb);
    }
    return NULL;
}

static void syncbuf_push_impl(SyncBuf* R sb, SyncBlock* R block, int opcode, const void* R p)
{
    block->opcode = opcode;
    if (p) memcpy(&block->data[0], p, sb->dataSize);
    atomic8_set(&block->hasBeenWritten, 1);
    
    /* Advance over completely written blocks, as far as possible */
    for (;;) {
        int next;
        int index = atomic32_get(&sb->writeStart);
        block = syncbuf_get_block(sb, index);
        
        /* If the hasBeenWritten flag has already been cleared, another thread must have already advanced for us
        ** Or, we've reached the end of what was available
        */
        if (!atomic8_cmp_xchg(&block->hasBeenWritten, 1, 0))
            break;
        
        next = index + 1;
        if (next == sb->slotCount)
            next = 0;
        
        /* Advance writeStart and loop */
        atomic32_cmp_xchg(&sb->writeStart, index, next);
    }
}

int syncbuf_push(SyncBuf* R sb, int opcode, const void* R p)
{
    for (;;) {
        int index = atomic32_get(&sb->writeEnd);
        int next = index + 1;
        
        if (next == sb->slotCount)
            next = 0;
        
        if (next == atomic32_get(&sb->readStart))
            return ZEQ_ERR_BOUNDS;
        
        if (atomic32_cmp_xchg(&sb->writeEnd, index, next)) {
            syncbuf_push_impl(sb, syncbuf_get_block(sb, index), opcode, p);
            return ZEQ_OK;
        }
    }
}

void syncbuf_push_force(SyncBuf* R sb, int opcode, const void* R data)
{
    for (;;) {
        int rc = syncbuf_push(sb, opcode, data);
        if (!rc) break;
    }
}

static void syncbuf_pop_impl(SyncBuf* R sb, SyncBlock* R block, int* R opcode, void* R p)
{
    if (opcode) *opcode = block->opcode;
    if (p) memcpy(p, &block->data[0], sb->dataSize);
    atomic8_set(&block->hasBeenRead, 1);
    
    /* Advance over completely read blocks, as far as possible */
    for (;;) {
        int next;
        int index = atomic32_get(&sb->readStart);
        block = syncbuf_get_block(sb, index);
        
        /* If the hasBeenRead flag has already been cleared, another thread must have already advanced for us
        ** Or, we've reached the end of what was available
        */
        if (!atomic8_cmp_xchg(&block->hasBeenRead, 1, 0))
            return;
        
        next = index + 1;
        if (next == sb->slotCount)
            next = 0;
        
        /* Advance readStart and loop */
        atomic32_cmp_xchg(&sb->readStart, index, next);
    }
}

int syncbuf_pop(SyncBuf* R sb, int* R opcode, void* R p)
{
    for (;;) {
        int index = atomic32_get(&sb->readEnd);
        int next;
        
        if (index == atomic32_get(&sb->writeStart))
            return ZEQ_ERR_AGAIN;
        
        next = index + 1;
        if (next == sb->slotCount)
            next = 0;
        
        if (atomic32_cmp_xchg(&sb->readEnd, index, next)) {
            syncbuf_pop_impl(sb, syncbuf_get_block(sb, index), opcode, p);
            return ZEQ_OK;
        }
    }
}

int syncbuf_sem_trigger(SyncBuf* sb)
{
    return semaphore_trigger(&sb->sem);
}

int syncbuf_sem_wait(SyncBuf* sb)
{
    return semaphore_wait(&sb->sem);
}

int syncbuf_sem_try_wait(SyncBuf* sb)
{
    return semaphore_try_wait(&sb->sem);
}
