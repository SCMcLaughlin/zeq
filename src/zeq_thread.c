
#include <pthread.h>
#include <stdlib.h>
#include "zeq_alloc.h"
#include "zeq_def.h"
#include "zeq_err.h"
#include "zeq_thread.h"

typedef struct {
    ThreadProc  func;
    void*       userdata;
} ThreadCreate;

static void* thread_proc_wrapper(void* ptr)
{
    ThreadCreate* tc = (ThreadCreate*)ptr;
    ThreadProc func = tc->func;
    void* userdata = tc->userdata;
    
    free(tc);
    func(userdata);
    return NULL;
}

int thread_start(ThreadProc func, void* userdata)
{
    ThreadCreate* tc = (ThreadCreate*)zeq_malloc(sizeof(ThreadCreate));
    pthread_t pthread;
    
    if (!tc) return ZEQ_ERR_MEMORY;
    
    tc->func = func;
    tc->userdata = userdata;
    
    if (pthread_create(&pthread, NULL, thread_proc_wrapper, tc) || pthread_detach(pthread)) {
        free(tc);
        return ZEQ_ERR_API;
    }
    
    return ZEQ_OK;
}
