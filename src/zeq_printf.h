
#ifndef ZEQ_PRINTF_H
#define ZEQ_PRINTF_H

#include <stdio.h>
#include "stb_sprintf.h"
#include "zeq_def.h"

#define zsnprintf stbsp_snprintf
#define zvsnprintf stbsp_vsnprintf
ZEQ_INTERFACE void zfprintf(FILE* fp, const char* fmt, ...);
#define zprintf(...) zfprintf(stdout, __VA_ARGS__)
ZEQ_INTERFACE int zaprintf(char** buf, const char* fmt, ...);
ZEQ_INTERFACE int zvaprintf(char** buf, const char* fmt, va_list args);

#endif/*ZEQ_PRINTF_H*/
