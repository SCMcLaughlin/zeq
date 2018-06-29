
#ifndef ZEQ_SEMAPHORE_H
#define ZEQ_SEMAPHORE_H

#include <stdint.h>
#include "zeq_def.h"
#include "zeq_platform.h"

#ifdef ZEQ_OS_WINDOWS
# include <windows.h>
typedef HANDLE Semaphore;

ZEQ_INTERFACE int semaphore_init(Semaphore* sem);
ZEQ_INTERFACE int semaphore_deinit(Semaphore* sem);
ZEQ_INTERFACE int semaphore_wait_win32(Semaphore sem);
ZEQ_INTERFACE int semaphore_try_wait_win32(Semaphore sem);
ZEQ_INTERFACE int semaphore_wait_with_timeout_win32(Semaphore ptr, uint32_t timeoutMs);
ZEQ_INTERFACE int semaphore_trigger_win32(Semaphore sem);
# define semaphore_wait(ptr) semaphore_wait_win32(*(ptr))
# define semaphore_try_wait(ptr) semaphore_try_wait_win32(*(ptr))
# define semaphore_wait_with_timeout(ptr, ms) semaphore_wait_with_timeout_win32(*(ptr), (ms))
# define semaphore_trigger(ptr) semaphore_trigger_win32(*(ptr))
#else
# include <semaphore.h>
typedef sem_t Semaphore;

ZEQ_INTERFACE int semaphore_init(Semaphore* sem);
ZEQ_INTERFACE int semaphore_deinit(Semaphore* sem);
ZEQ_INTERFACE int semaphore_wait(Semaphore* sem);
ZEQ_INTERFACE int semaphore_try_wait(Semaphore* sem);
ZEQ_INTERFACE int semaphore_wait_with_timeout(Semaphore* ptr, uint32_t timeoutMs);
ZEQ_INTERFACE int semaphore_trigger(Semaphore* sem);
#endif

#endif/*ZEQ_SEMAPHORE_H*/
