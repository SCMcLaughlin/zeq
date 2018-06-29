
#include <stdarg.h>
#include <stdio.h>
#include "zeq_alloc.h"
#include "zeq_def.h"
#include "zeq_err.h"
#include "zeq_printf.h"

#define STB_SPRINTF_IMPLEMENTATION
# include "stb_sprintf.h"
#undef STB_SPRINTF_IMPLEMENTATION

static char* zfprintf_cb(char* buf, void* userdata, int len)
{
    fwrite(buf, len, 1, (FILE*)userdata);
    return buf;
}

void zfprintf(FILE* fp, const char* fmt, ...)
{
    char buf[STB_SPRINTF_MIN];
    va_list args;
    
    va_start(args, fmt);
    stbsp_vsprintfcb(zfprintf_cb, fp, buf, fmt, args);
    va_end(args);
}

int zaprintf(char** buf, const char* fmt, ...)
{
    va_list args;
    int rc;
    
    va_start(args, fmt);
    rc = zvaprintf(buf, fmt, args);
    va_end(args);
    return rc;
}

int zvaprintf(char** buf, const char* fmt, va_list args)
{
    va_list check;
    char* ptr;
    int rc;
    
    if (!buf) return ZEQ_ERR_INVALID;
    
    va_copy(check, args);
    rc = zvsnprintf(NULL, 0, fmt, check);
    va_end(check);
    if (rc <= 0) return ZEQ_ERR_API;
    
    rc++;
    ptr = (char*)zeq_realloc(*buf, (size_t)rc);
    if (ptr) {
        rc = zvsnprintf(ptr, rc, fmt, args);
        if (rc <= 0) {
            free(ptr);
            return ZEQ_ERR_API;
        }
        *buf = ptr;
        return rc;
    } else {
        if (*buf) {
            free(*buf);
            *buf = NULL;
        }
    }
    return ZEQ_ERR_API;
}
