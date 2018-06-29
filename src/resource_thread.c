
#include <limits.h>
#include <string.h>
#include "hash.h"
#include "load_scheduler.h"
#include "resource_op.h"
#include "resource_thread.h"
#include "syncbuf.h"
#include "zeq_alloc.h"
#include "zeq_err.h"
#include "zeq_thread.h"
#include "zone_load.h"

#define RES_QUEUE_SIZE 1024

struct IrZone;

typedef struct ResLoadZone {
    ZoneLoad*       load;
    struct IrZone*  irZone;
    LoadScheduler   sched;
} ResLoadZone;

typedef struct ResThread {
    uint8_t         zoneLoadCount;
    uint8_t         zoneLoadCapacity;
    ResLoadZone*    zoneLoads;
    SyncBuf*        resQueue;
    SyncBuf*        resultQueue;
    SyncBuf*        workQueue;
    char*           dirPath;
} ResThread;

static void res_thread_destroy(ResThread* rt)
{
    rt->resQueue = syncbuf_destroy(rt->resQueue);
    free(rt);
}

static void res_handle_zone_request(ResThread* rt, ResPacket* rp)
{
    ResLoadZone* rz;
    ZoneLoad* zl;
    int rc;
    
    if (rt->zoneLoadCount >= rt->zoneLoadCapacity) {
        uint32_t cap = (rt->zoneLoadCapacity == 0) ? 1 : rt->zoneLoadCapacity * 2;
        ResLoadZone* zoneLoads;
        
        if (cap > UCHAR_MAX) {
            /*fixme: report*/
            return;
        }
        
        zoneLoads = (ResLoadZone*)zeq_realloc(rt->zoneLoads, sizeof(ResLoadZone) * cap);
        if (!zoneLoads) {
            /*fixme: report*/
            return;
        }
        
        rt->zoneLoadCapacity = (uint8_t)cap;
        rt->zoneLoads = zoneLoads;
    }
    
    zl = (ZoneLoad*)zeq_calloc(1, sizeof(ZoneLoad));
    if (!zl) {
        /*fixme: report*/
        return;
    }
    
    zl->shortName = (char*)rp->data;
    zl->dirPath = rt->dirPath;
    
    rz = &rt->zoneLoads[rt->zoneLoadCount++];
    memset(rz, 0, sizeof(ResLoadZone));
    rz->load = zl;
    
    rc = zone_load_determine_type(rt->workQueue, rt->resQueue, zl);
    if (rc) {
        /*fixme: report error*/
        rt->zoneLoadCount--;
        return;
    }
}

static ResLoadZone* res_load_zone_by_zl(ResThread* rt, ZoneLoad* zl)
{
    uint32_t i;
    
    for (i = 0; i < rt->zoneLoadCount; i++) {
        ResLoadZone* rz = &rt->zoneLoads[i];
        
        if (rz->load == zl)
            return rz;
    }
    
    return NULL;
}

static void res_handle_zone_type_wld(ResThread* rt, ResPacket* rp)
{
    ZoneLoad* zl = (ZoneLoad*)rp->data;
    ResLoadZone* rz = res_load_zone_by_zl(rt, zl);
    int rc;
    
    /*fixme: assert on rz != NULL*/
    
    rc = load_sched_init_wld_zone(&rz->sched, rt->workQueue, rt->resQueue, zl);
    if (rc) {
        /*fixme: report and handle*/
    }
}

static void res_handle_zone_load_progress(ResThread* rt, ResPacket* rp)
{
    LoadPacketZone* lp = (LoadPacketZone*)rp->data;
    ResLoadZone* rz = res_load_zone_by_zl(rt, lp->input);
    int rc;
    
    rc = load_sched_zone_task_passed(&rz->sched, lp->taskIndex, rt->workQueue, rt->resQueue, lp->input);
    if (rc < 0) {
        /*fixme: report*/
    } else if (rc == ZEQ_DONE) {
        /*fixme: handle the zone load being finished*/
    }
    
    free(lp);
}

static int res_handle_packet(ResThread* rt, int op, ResPacket* rp)
{
    switch (op) {
    case RES_OP_SHUTDOWN:
        return ZEQ_DONE;
    
    case RES_OP_ZONE_REQUEST:
        res_handle_zone_request(rt, rp);
        break;
    
    case RES_OP_ZONE_TYPE_WLD:
        res_handle_zone_type_wld(rt, rp);
        break;
    
    case RES_OP_ZONE_TYPE_EQG:
        break;
    
    case RES_OP_ZONE_TYPE_NOT_FOUND:
        break;
    
    case RES_OP_ZONE_LOAD_PROGRESS:
        break;
    
    default:
        break;
    }
    
    return ZEQ_OK;
}

static void res_thread_proc(void* ptr)
{
    ResThread* rt = (ResThread*)ptr;
    ResPacket rp;
    int op, rc;
    
    for (;;) {
        syncbuf_sem_wait(rt->resQueue);
        
        for (;;) {
            rc = syncbuf_pop(rt->resQueue, &op, &rp);
            if (rc) break;
            rc = res_handle_packet(rt, op, &rp);
            if (rc) goto done;
        }
    }
    
done:
    res_thread_destroy(rt);
}

SyncBuf* resource_thread_start(SyncBuf* workQueue, SyncBuf* resultQueue, char* dirPath)
{
    ResThread* rt = (ResThread*)zeq_calloc(1, sizeof(ResThread));
    int rc;
    
    if (!rt) return NULL;
    
    rt->resQueue = syncbuf_create(RES_QUEUE_SIZE, sizeof(ResPacket));
    if (!rt->resQueue) {
        free(rt);
        return NULL;
    }
    
    rt->resultQueue = resultQueue;
    rt->workQueue = workQueue;
    rt->dirPath = dirPath;
    
    rc = thread_start(res_thread_proc, rt);
    if (rc) {
        res_thread_destroy(rt);
        return NULL;
    }
    
    return rt->resultQueue;
}
