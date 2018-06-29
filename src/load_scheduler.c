
#include <stddef.h>
#include <stdint.h>
#include "load_scheduler.h"
#include "syncbuf.h"
#include "work_queue.h"
#include "zeq_alloc.h"
#include "zeq_def.h"
#include "zeq_err.h"
#include "zone_load_wld.h"

#define LOAD_ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

enum LoadTaskState {
    LOAD_TASK_STATE_PENDING,
    LOAD_TASK_STATE_EXECUTING,
    LOAD_TASK_STATE_PASSED,
    LOAD_TASK_STATE_FAILED,
};

struct ZoneLoad;

typedef struct LoadTask {
    const uint32_t  depCount;
    WorkFunc        func;
    const uint32_t* deps;
} LoadTask;

enum LoadTaskZoneWld {
    /* Phase 1 */
    LOAD_TASK_ZONE_WLD_BASIC_GEOMETRY,
    LOAD_TASK_ZONE_WLD_OBJECT_GEOMETRY,
    LOAD_TASK_ZONE_WLD_OBJECT_POSITIONS,
    LOAD_TASK_ZONE_WLD_LIGHTS,
    /* Phase 2 */
    LOAD_TASK_ZONE_WLD_OBJECT_PLACEMENT,
    LOAD_TASK_ZONE_WLD_TEXTURES,
    /* Phase 3 */
    LOAD_TASK_ZONE_WLD_OCTREE
};

static const uint32_t loadDepsZoneWldObjectPlacement[] = {
    LOAD_TASK_ZONE_WLD_BASIC_GEOMETRY,
    LOAD_TASK_ZONE_WLD_OBJECT_GEOMETRY,
    LOAD_TASK_ZONE_WLD_OBJECT_POSITIONS
};

static const uint32_t loadDepsZoneWldTextures[] = {
    LOAD_TASK_ZONE_WLD_BASIC_GEOMETRY,
    LOAD_TASK_ZONE_WLD_OBJECT_GEOMETRY
};

static const uint32_t loadDepsZoneWldOctree[] = {
    LOAD_TASK_ZONE_WLD_BASIC_GEOMETRY,
    LOAD_TASK_ZONE_WLD_OBJECT_GEOMETRY,
    LOAD_TASK_ZONE_WLD_OBJECT_POSITIONS,
    LOAD_TASK_ZONE_WLD_OBJECT_PLACEMENT
};

static const LoadTask loadTasksZoneWld[] = {
    /* LOAD_TASK_ZONE_WLD_BASIC_GEOMETRY */
    {
        0,
        zone_load_wld_basic_geometry,
        NULL
    },
    /* LOAD_TASK_ZONE_WLD_OBJECT_GEOMETRY */
    {
        0,
        NULL,
        NULL
    },
    /* LOAD_TASK_ZONE_WLD_OBJECT_POSITIONS */
    {
        0,
        NULL,
        NULL
    },
    /* LOAD_TASK_ZONE_WLD_LIGHTS */
    {
        0,
        NULL,
        NULL
    },
    /* LOAD_TASK_ZONE_WLD_OBJECT_PLACEMENT */
    {
        LOAD_ARRAY_SIZE(loadDepsZoneWldObjectPlacement),
        NULL,
        loadDepsZoneWldObjectPlacement
    },
    /* LOAD_TASK_ZONE_WLD_TEXTURES */
    {
        LOAD_ARRAY_SIZE(loadDepsZoneWldTextures),
        NULL,
        loadDepsZoneWldTextures
    },
    /* LOAD_TASK_ZONE_WLD_OCTREE */
    {
        LOAD_ARRAY_SIZE(loadDepsZoneWldOctree),
        NULL,
        loadDepsZoneWldOctree
    }
};

void load_sched_deinit(LoadScheduler* sched)
{
    if (sched->taskStates) {
        free(sched->taskStates);
        sched->taskStates = NULL;
    }
}

static int load_sched_zone(LoadScheduler* sched, const LoadTask* task, SyncBuf* workQueue, SyncBuf* resultQueue, struct ZoneLoad* zl)
{
    uint32_t taskIndex = (uint32_t)(task - sched->tasks);
    LoadPacketZone* packet = (LoadPacketZone*)zeq_malloc(sizeof(LoadPacketZone));
    int rc;
    
    if (!packet) return ZEQ_ERR_MEMORY;
    
    packet->taskIndex = taskIndex;
    packet->rc = ZEQ_OK;
    packet->output = NULL;
    packet->input = zl;
    
    rc = work_queue(workQueue, resultQueue, task->func, packet);
    if (rc) return rc;
    
    sched->taskStates[taskIndex] = LOAD_TASK_STATE_EXECUTING;
    return ZEQ_OK;
}

int load_sched_init_wld_zone(LoadScheduler* sched, SyncBuf* workQueue, SyncBuf* resultQueue, struct ZoneLoad* zl)
{
    uint32_t i;
    int rc;
    
    sched->taskCount = LOAD_ARRAY_SIZE(loadTasksZoneWld);
    sched->taskStates = (uint8_t*)zeq_calloc(LOAD_ARRAY_SIZE(loadTasksZoneWld), sizeof(uint8_t));
    sched->tasks = loadTasksZoneWld;
    
    if (!sched->taskStates) return ZEQ_ERR_MEMORY;
    
    /* Scan for tasks with no dependencies and queue them right away */
    for (i = 0; i < sched->taskCount; i++) {
        const LoadTask* task = &sched->tasks[i];
        
        if (task->depCount != 0)
            continue;
        
        rc = load_sched_zone(sched, task, workQueue, resultQueue, zl);
        if (rc) return rc;
    }
    
    return ZEQ_OK;
}

static ZEQ_ALWAYS_INLINE int load_sched_all_passed(LoadScheduler* sched)
{
    uint32_t i;
    
    for (i = 0; i < sched->taskCount; i++) {
        if (sched->taskStates[i] != LOAD_TASK_STATE_PASSED)
            return 0;
    }
    
    return 1;
}

static ZEQ_ALWAYS_INLINE int load_sched_deps_met(LoadScheduler* sched, const LoadTask* task)
{
    uint32_t i;
    
    for (i = 0; i < task->depCount; i++) {
        if (sched->taskStates[task->deps[i]] != LOAD_TASK_STATE_PASSED)
            return 0;
    }
    
    return 1;
}

int load_sched_zone_task_passed(LoadScheduler* sched, uint32_t taskIndex, SyncBuf* workQueue, SyncBuf* resultQueue, struct ZoneLoad* zl)
{
    uint32_t i;
    int rc;
    
    sched->taskStates[taskIndex] = LOAD_TASK_STATE_PASSED;
    
    /* Check if we've finished everything */
    if (load_sched_all_passed(sched))
        return ZEQ_DONE;
    
    /* Check if any pending tasks now have all of their dependencies met */
    for (i = 0; i < sched->taskCount; i++) {
        if (sched->taskStates[i] != LOAD_TASK_STATE_PENDING)
            continue;
        
        if (load_sched_deps_met(sched, &sched->tasks[i])) {
            rc = load_sched_zone(sched, &sched->tasks[i], workQueue, resultQueue, zl);
            if (rc) return rc;
        }
    }
    
    return ZEQ_OK;
}
