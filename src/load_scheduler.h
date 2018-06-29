
#ifndef LOAD_SCHEDULER_H
#define LOAD_SCHEDULER_H

#include <stdint.h>
#include "zeq_def.h"

struct IrZone;
struct LoadTask;
struct SyncBuf;
struct ZoneLoad;

typedef struct LoadScheduler {
    uint32_t                taskCount;
    uint8_t*                taskStates;
    const struct LoadTask*  tasks;
} LoadScheduler;

typedef struct LoadPacketZone {
    uint32_t            taskIndex;
    int                 rc;
    struct IrZone*      output;
    struct ZoneLoad*    input;
} LoadPacketZone;

ZEQ_INTERFACE void load_sched_deinit(LoadScheduler* sched);
ZEQ_INTERFACE int load_sched_init_wld_zone(LoadScheduler* sched, struct SyncBuf* workQueue, struct SyncBuf* resultQueue, struct ZoneLoad* zl);
ZEQ_INTERFACE int load_sched_zone_task_passed(LoadScheduler* sched, uint32_t taskIndex, struct SyncBuf* workQueue, struct SyncBuf* resultQueue, struct ZoneLoad* zl);

#endif/*LOAD_SCHEDULER_H*/
