
#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include "zeq_def.h"

ZEQ_INTERFACE uint32_t hash_string(const char* str, uint32_t len);
ZEQ_INTERFACE uint32_t hash_double(double value);
ZEQ_INTERFACE uint32_t hash_int64(int64_t value);

#endif/*HASH_H*/
