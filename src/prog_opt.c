
#include <stdint.h>
#include <string.h>
#include "prog_opt.h"

static uint32_t progOptState = 1;

static int prog_opt_long(const char* str, const ProgOptLong* longopts, int* index)
{
    const ProgOptLong* cur = longopts;
    size_t len = strlen(str);
    int rc;
    
    for (;;) {
        if (cur->name == NULL)
            return PROG_OPT_UNKNOWN;
        
        if (strncmp(str, cur->name, len) == 0) {
            if (cur->flag) {
                *cur->flag = cur->value;
                rc = PROG_OPT_FLAG;
                break;
            }
            
            rc = cur->value;
            break;
        }
        
        cur++;
    }
    
    *index = (int)(cur - longopts);
    return rc;
}

int prog_opt(int argc, char** argv, const ProgOptLong* longopts, int* index)
{
    uint32_t minor = ((progOptState) & 0xff000000) >> 24;
    uint32_t i = (progOptState) & 0x00ffffff;
    int c;
    
    *index = -1;
    
    if (minor != 0) {
        c = argv[i][minor + 1];
        
        if (c != '\0') {
            minor++;
            goto done_no_advance;
        }
        
        minor = 0;
        i++;
    }
    
    if (i >= (uint32_t)argc)
        return PROG_OPT_END;
    
    c = argv[i][0];
    
    if (c != '-') {
        c = PROG_OPT_ARG;
        *index = (int)i;
        goto done;
    }
    
    c = argv[i][1];
    
    if (c != '-') {
        minor++;
        goto done_no_advance;
    }
    
    c = prog_opt_long(&argv[i][2], longopts, index);
    
    if (c == PROG_OPT_UNKNOWN) {
        *index = (int)i;
        goto done;
    }
    
done:
    i++;
done_no_advance:
    progOptState = (minor << 24) | i;
    return c;
}
