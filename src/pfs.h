
#ifndef PFS_H
#define PFS_H

#include <stdint.h>
#include "zeq_byte.h"
#include "zeq_def.h"

typedef struct Pfs Pfs;

ZEQ_INTERFACE int pfs_open(Pfs** pfs, const char* path);
ZEQ_INTERFACE Pfs* pfs_close(Pfs* pfs);

ZEQ_INTERFACE uint32_t pfs_file_count(Pfs* pfs);
ZEQ_INTERFACE const char* pfs_file_name(Pfs* pfs, uint32_t index);
ZEQ_INTERFACE int pfs_file_data(Pfs* pfs, uint32_t index, byte** data, uint32_t* length);
ZEQ_INTERFACE int pfs_file_data_by_name(Pfs* pfs, const char* name, uint32_t namelen, byte** data, uint32_t* length);

#endif/*PFS_H*/
