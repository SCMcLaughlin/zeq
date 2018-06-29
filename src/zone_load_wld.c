
#include <stdarg.h>
#include "load_scheduler.h"
#include "pfs.h"
#include "resource_op.h"
#include "wld.h"
#include "wld_types.h"
#include "work_queue.h"
#include "zeq_byte.h"
#include "zeq_err.h"
#include "zeq_printf.h"
#include "zone_load.h"
#include "zone_load_wld.h"

struct SyncBuf;

static int zlw_open_wld(Pfs* pfs, Wld** wld, const char* name, uint32_t namelen)
{
    byte* data;
    uint32_t len;
    int rc;
    
    rc = pfs_file_data_by_name(pfs, name, namelen, &data, &len);
    if (rc) return rc;
    
    return wld_open(wld, data, len);
}

static int zlw_open_wld_format(Pfs* pfs, Wld** wld, const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    int len;
    
    va_start(args, fmt);
    len = zvsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    if (len <= 0 || len >= (int)sizeof(buf))
        return ZEQ_ERR_STRING;
    
    return zlw_open_wld(pfs, wld, buf, (uint32_t)len);
}

static int zlw_static_mesh(WldFrag* frag)
{
    WldFragMesh* mesh = (WldFragMesh*)frag;
    return ZEQ_OK;
}

void zone_load_wld_basic_geometry(void* ptr, struct SyncBuf* resultQueue)
{
    LoadPacketZone* packet = (LoadPacketZone*)ptr;
    ZoneLoad* zl = packet->input;
    uint8_t* fragTypes;
    WldFrag** frags;
    uint32_t count, i;
    int rc;
    
    rc = zlw_open_wld_format(zl->pfs, &zl->wld.wldMain, "%s.wld", zl->shortName);
    if (rc) goto fail;
    
    fragTypes = wld_frag_types(zl->wld.wldMain, &count);
    if (!fragTypes || count == 0)
        goto done;
    frags = wld_frags(zl->wld.wldMain, NULL);
    
    for (i = 0; i < count; i++) {
        if (fragTypes[i] != WLD_FRAG_MESH)
            continue;
        
        rc = zlw_static_mesh(frags[i]);
        if (rc) goto fail;
    }
    
fail:
    packet->rc = rc;
done:
    work_finish(resultQueue, RES_OP_ZONE_LOAD_PROGRESS, packet);
}
