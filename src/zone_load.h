
#ifndef ZONE_LOAD_H
#define ZONE_LOAD_H

#include <stdint.h>
#include "zeq_def.h"

struct IrZone;
struct Pfs;
struct Wld;
struct SyncBuf;

typedef struct ZoneLoad {
    int                     rc;         /* Only used for zone_load_determine_type() errors */
    char*                   shortName;
    char*                   dirPath;
    struct Pfs*             pfs;
    const struct IrZone*    cur;
    union {
        struct {
            struct Pfs*     pfsObjDef;
            struct Pfs*     pfsObjDef2;
            struct Wld*     wldMain;    /* From main pfs: <shortname>.wld */
            struct Wld*     wldLights;  /* From main pfs: lights.wld */
            struct Wld*     wldObjDef;  /* From pfsObjDef: <shortname>_obj.wld */
            struct Wld*     wldObjDef2; /* From pfsObjDef2: <shortname>_2_obj.wld */
            struct Wld*     wldObjLoc;  /* From main pfs: objects.wld */
        } wld;
        struct {
            
        } eqg;
    };
} ZoneLoad;

ZEQ_INTERFACE int zone_load_determine_type(struct SyncBuf* workQueue, struct SyncBuf* resultQueue, ZoneLoad* zl);

#endif/*ZONE_LOAD_H*/
