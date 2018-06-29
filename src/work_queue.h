
#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <stdint.h>
#include "zeq_def.h"

struct SyncBuf;

typedef void(*WorkFunc)(void* data, struct SyncBuf* resultQueue);

ZEQ_INTERFACE int work_threads_start(struct SyncBuf* workQueue, uint32_t n);
ZEQ_INTERFACE int work_threads_shutdown(struct SyncBuf* workQueue, uint32_t n, int64_t timeoutMs);
ZEQ_INTERFACE struct SyncBuf* work_queue_create();
ZEQ_INTERFACE int work_queue(struct SyncBuf* workQueue, struct SyncBuf* resultQueue, WorkFunc func, void* data);
ZEQ_INTERFACE int work_finish(struct SyncBuf* resultQueue, int op, void* data);

#endif/*WORK_QUEUE_H*/
