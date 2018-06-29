
#ifndef ZEQ_CLOCK_H
#define ZEQ_CLOCK_H

#include <stdint.h>
#include "zeq_def.h"

ZEQ_INTERFACE int64_t clock_milliseconds();
ZEQ_INTERFACE int64_t clock_microseconds();
ZEQ_INTERFACE int64_t clock_unix_seconds();
ZEQ_INTERFACE void clock_sleep(uint32_t ms);

#endif/*ZEQ_CLOCK_H*/
