
#include <stdint.h>
#include "syncbuf.h"
#include "work_queue.h"
#include "zeq_alloc.h"
#include "zeq_err.h"
#include "zeq_platform.h"
#include "zeq_thread.h"

#define WORK_QUEUE_SIZE 2048

typedef struct WorkPacket {
    WorkFunc    func;
    void*       data;
    SyncBuf*    resultQueue;
} WorkPacket;

typedef struct WorkThread {
    SyncBuf*    workQueue;
} WorkThread;

static uint32_t work_processor_thread_count()
{
#if defined(ZEQ_OS_WINDOWS)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
    return (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
#else
    return 0;
#endif
}

static uint32_t work_thread_count(uint32_t n)
{
    if (n == 0) {
        n = work_processor_thread_count();
        if (n == 0)
            n = 1;
    }
    return n;
}

static void work_thread_proc(void* ptr)
{
    WorkThread* wt = (WorkThread*)ptr;
    WorkPacket wp;
    int rc;
    
    for (;;) {
        syncbuf_sem_wait(wt->workQueue);
        
        for (;;) {
            rc = syncbuf_pop(wt->workQueue, NULL, &wp);
            if (rc) break;
            
            if ((intptr_t)wp.func == 1) {
                /*fixme: send reply?*/
                return;
            }
            
            wp.func(wp.data, wp.resultQueue);
        }
    }
}

int work_threads_start(SyncBuf* workQueue, uint32_t n)
{
    uint32_t i;
    int rc;
    
    n = work_thread_count(n);
    for (i = 0; i < n; i++) {
        WorkThread* wt = (WorkThread*)zeq_malloc(sizeof(WorkThread));
        
        if (!wt) return ZEQ_ERR_MEMORY;
        wt->workQueue = workQueue;
        
        rc = thread_start(work_thread_proc, wt);
        if (rc) return rc;
    }
    
    return ZEQ_OK;
}

int work_threads_shutdown(SyncBuf* workQueue, uint32_t n, int64_t timeoutMs)
{
    uint32_t i;
    
    /*fixme*/
    (void)timeoutMs;
    
    n = work_thread_count(n);
    for (i = 0; i < n; i++) {
        work_queue(workQueue, NULL, (WorkFunc)1, NULL);
    }
    
    return ZEQ_OK;
}

SyncBuf* work_queue_create()
{
    return syncbuf_create(WORK_QUEUE_SIZE, sizeof(WorkPacket));
}

int work_queue(SyncBuf* workQueue, SyncBuf* resultQueue, WorkFunc func, void* data)
{
    WorkPacket wp;
    
    wp.func = func;
    wp.data = data;
    wp.resultQueue = resultQueue;
    
    syncbuf_push_force(workQueue, 0, &wp);
    return syncbuf_sem_trigger(workQueue);
}

int work_finish(SyncBuf* resultQueue, int op, void* data)
{
    syncbuf_push_force(resultQueue, op, data);
    return syncbuf_sem_trigger(resultQueue);
}
