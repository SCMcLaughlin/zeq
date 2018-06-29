
#include <stdint.h>
#include "pfs.h"
#include "resource_op.h"
#include "resource_thread.h"
#include "syncbuf.h"
#include "work_queue.h"
#include "zeq_alloc.h"
#include "zeq_err.h"
#include "zeq_printf.h"
#include "zone_load.h"

static void zone_load_determine_type_cb(void* ptr, SyncBuf* resultQueue)
{
    ZoneLoad* zl = (ZoneLoad*)ptr;
    ResPacket rp;
    char buf[1024];
    int len, rc, op;
    
    /* EQG zones take precedence over WLD zones */
    len = zsnprintf(buf, sizeof(buf), "%s/%s.eqg", zl->dirPath, zl->shortName);
    if (len <= 0 || len >= (int)sizeof(buf)) {
        zl->rc = ZEQ_ERR_STRING;
        goto send_error;
    }
    
    rc = pfs_open(&zl->pfs, buf);
    if (rc) {
        if (rc != ZEQ_ERR_NOT_FOUND) {
            zl->rc = rc;
            goto send_error;
        }
    } else {
        op = RES_OP_ZONE_TYPE_EQG;
        goto send_result;
    }
    
    len = zsnprintf(buf, sizeof(buf), "%s/%s.s3d", zl->dirPath, zl->shortName);
    if (len <= 0 || len >= (int)sizeof(buf)) {
        zl->rc = ZEQ_ERR_STRING;
        goto send_error;
    }
    
    rc = pfs_open(&zl->pfs, buf);
    if (rc) {
        if (rc != ZEQ_ERR_NOT_FOUND) {
            zl->rc = rc;
            goto send_error;
        }
    } else {
        op = RES_OP_ZONE_TYPE_WLD;
        goto send_result;
    }
    
send_error:
    op = RES_OP_ZONE_TYPE_NOT_FOUND;
send_result:
    rp.data = zl;
    work_finish(resultQueue, op, &rp);
}

int zone_load_determine_type(SyncBuf* workQueue, SyncBuf* resultQueue, ZoneLoad* zl)
{
    return work_queue(workQueue, resultQueue, zone_load_determine_type_cb, zl);
}
