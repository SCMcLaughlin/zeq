
#include <ctype.h>
#include <string.h>
#include "zeq_string.h"

void str2lower(char* str, int len)
{
    /* Only lowercases ascii characters, ignores utf-8 characters */
    int i;
    if (len < 0) len = strlen(str);
    for (i = 0; i < len; i++) {
        int c = str[i];
        if (c >= 'A' && c <= 'Z')
            str[i] = c + 32; /* 'A' -> 'a' */
    }
}
