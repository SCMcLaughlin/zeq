
#ifndef RESOURCE_THREAD_H
#define RESOURCE_THREAD_H

#include "zeq_def.h"

struct SyncBuf;

typedef struct ResPacket {
    void*   data;
} ResPacket;

ZEQ_INTERFACE struct SyncBuf* resource_thread_start(struct SyncBuf* workQueue, struct SyncBuf* resultQueue, char* dirPath);

#endif/*RESOURCE_THREAD_H*/
