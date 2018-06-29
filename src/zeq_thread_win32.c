
#include <process.h>
#include <windows.h>
#include "zeq_alloc.h"
#include "zeq_def.h"
#include "zeq_err.h"
#include "zeq_thread.h"

typedef struct {
    ThreadProc  func;
    void*       userdata;
} ThreadCreate;

static unsigned int WINAPI thread_proc_wrapper(void* ptr)
{
    ThreadCreate* tc = (ThreadCreate*)ptr;
    ThreadProc func = tc->func;
    void* userdata = tc->userdata;
    
    free(tc);
    func(userdata);
    _endthreadex(0);
    return 0;
}

int thread_start(ThreadProc func, void* userdata)
{
    ThreadCreate* tc = (ThreadCreate*)zeq_malloc(sizeof(ThreadCreate));
    HANDLE handle;
    
    if (!tc) return ZEQ_ERR_MEMORY;
    
    tc->func = func;
    tc->userdata = userdata;
    
    handle = (HANDLE)_beginthreadex(NULL, 0, thread_proc_wrapper, tc, 0, NULL);
    
    if (!handle || handle == INVALID_HANDLE_VALUE) {
        free(tc);
        return ZEQ_ERR_API;
    }
    
    CloseHandle(handle);
    return ZEQ_OK;
}
