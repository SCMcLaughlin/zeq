
#ifndef SYNCBUF_H
#define SYNCBUF_H

#include <limits.h>
#include <stdint.h>
#include "zeq_def.h"

#define SYNCBUF_MIN_PACKETS 2
#define SYNCBUF_MAX_PACKETS (INT_MAX / 2)

typedef struct SyncBuf SyncBuf;

ZEQ_INTERFACE SyncBuf* syncbuf_create(uint32_t slotCount, uint32_t dataSize);
ZEQ_INTERFACE SyncBuf* syncbuf_destroy(SyncBuf* sb);
ZEQ_INTERFACE int syncbuf_push(SyncBuf* R sb, int opcode, const void* R data);
ZEQ_INTERFACE void syncbuf_push_force(SyncBuf* R sb, int opcode, const void* R data);
ZEQ_INTERFACE int syncbuf_pop(SyncBuf* R sb, int* R opcode, void* R data);
ZEQ_INTERFACE int syncbuf_sem_trigger(SyncBuf* sb);
ZEQ_INTERFACE int syncbuf_sem_wait(SyncBuf* sb);
ZEQ_INTERFACE int syncbuf_sem_try_wait(SyncBuf* sb);

#endif/*SYNCBUF_H*/
