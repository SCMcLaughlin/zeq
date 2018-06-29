
#include <errno.h>
#include <semaphore.h>
#include <stdint.h>
#include "zeq_err.h"
#include "zeq_semaphore.h"

int semaphore_init(Semaphore* ptr)
{
    return sem_init(ptr, 0, 0) ? ZEQ_ERR_API : ZEQ_OK;
}

int semaphore_deinit(Semaphore* ptr)
{
    return sem_destroy(ptr) ? ZEQ_ERR_INVALID : ZEQ_OK;
}

int semaphore_wait(Semaphore* ptr)
{
    return sem_wait(ptr) ? ZEQ_ERR_API : ZEQ_OK;
}

int semaphore_try_wait(Semaphore* ptr)
{
    if (sem_trywait(ptr)) {
        int err = errno;
        return (err == EAGAIN) ? ZEQ_ERR_WOULD_BLOCK : ZEQ_ERR_API;
    }
    
    return ZEQ_OK;
}

int semaphore_wait_with_timeout(Semaphore* ptr, uint32_t timeoutMs)
{
    struct timespec t;
    t.tv_sec = timeoutMs / 1000;
    t.tv_nsec = (timeoutMs % 1000) * 1000000;
    
    if (sem_timedwait(ptr, &t)) {
        int err = errno;
        return (err == EAGAIN) ? ZEQ_ERR_WOULD_BLOCK : ZEQ_ERR_API;
    }
    
    return ZEQ_OK;
}

int semaphore_trigger(Semaphore* ptr)
{
    /* sem_post() has two possible errors:
    ** EINVAL: the semaphore is invalid
    ** EOVERFLOW: the semaphore's int would overflow
    ** 
    ** We don't care about EOVERFLOW; it will still be waking threads
    ** as long as the value is anything above 0, which is all we want
    */
    if (sem_post(ptr)) {
        int err = errno;
        if (err == EINVAL)
            return ZEQ_ERR_INVALID;
    }
    
    return ZEQ_OK;
}
