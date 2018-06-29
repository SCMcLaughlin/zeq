
#include <stdint.h>
#include <windows.h>
#include "zeq_err.h"
#include "zeq_semaphore.h"

int semaphore_init(Semaphore* ptr)
{
    *ptr = CreateSemaphore(NULL, 0, INT_MAX, NULL);
    return (*ptr == NULL) ? ZEQ_ERR_API : ZEQ_OK;
}

int semaphore_deinit(Semaphore* ptr)
{
    HANDLE h = *ptr;
    *ptr = NULL;
    
    return (h && !CloseHandle(h)) ? ZEQ_ERR_API : ZEQ_OK;
}

int semaphore_wait_win32(Semaphore ptr)
{
    return WaitForSingleObject(ptr, INFINITE) ? ZEQ_ERR_API : ZEQ_OK;
}

int semaphore_try_wait_win32(Semaphore ptr)
{
    DWORD ret = WaitForSingleObject(ptr, 0);
    
    switch (ret) {
    case WAIT_OBJECT_0:
        return ZEQ_OK;

    case WAIT_TIMEOUT:
        return ZEQ_ERR_WOULD_BLOCK;

    default:
        return ZEQ_ERR_API;
    }
}

int semaphore_wait_with_timeout_win32(Semaphore ptr, uint32_t timeoutMs)
{
    DWORD ret = WaitForSingleObject(ptr, timeoutMs);

    switch (ret) {
    case WAIT_OBJECT_0:
        return ZEQ_OK;

    case WAIT_TIMEOUT:
        return ZEQ_ERR_WOULD_BLOCK;

    default:
        return ZEQ_ERR_API;
    }
}

int semaphore_trigger_win32(Semaphore ptr)
{
    return ReleaseSemaphore(ptr, 1, NULL) ? ZEQ_OK : ZEQ_ERR_API;
}
