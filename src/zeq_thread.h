
#ifndef ZEQ_THREAD_H
#define ZEQ_THREAD_H

#include "zeq_def.h"

typedef void(*ThreadProc)(void* userdata);

ZEQ_INTERFACE int thread_start(ThreadProc func, void* userdata);

#endif/*ZEQ_THREAD_H*/
