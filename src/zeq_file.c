
#include <stdio.h>
#include <stdlib.h>
#include "zeq_alloc.h"
#include "zeq_file.h"

char* file2str(const char* path, size_t* len)
{
    FILE* fp = fopen(path, "rb");
    size_t bytes;
    char* ptr = NULL;
    
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    bytes = (size_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (bytes == 0) goto fail;
    
    ptr = (char*)zeq_malloc(bytes + 1);
    if (!ptr) goto fail;
    if (fread(ptr, bytes, 1, fp) != 1) {
        free(ptr);
        goto fail;
    }
    
    if (len) *len = bytes;
    
fail:
    fclose(fp);
    return ptr;
}
